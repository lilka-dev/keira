#include "web.h"
#include "esp_http_server.h"
#include <keira/servicemanager.h>

static const char html[] = R"(
<!DOCTYPE html>
<html>
<head>
  <title>Web services</title>
</head>
<body>
  <div style="display:flex;flex-direction:column;gap:1em;">
    <form action="/firmware" method="post" enctype="multipart/form-data">
      <input type="file" name="file">
      <button type="submit">Flash OTA & Run</button>
    </form>
    <a href="/download?sd=true"/>Browse SD card</a>
    <a href="/download"/>Browse SPIFS</a>
  </div>
</body>
</html>)";

static const char htmlListBegin[] =
    R"(<!DOCTYPE html><html><meta charset="UTF-8"><body><ul style="list-style: none;">)";
static const char htmlListPageEnd[] = "</ul></body></html>";

static httpd_handle_t stream_httpd = NULL;
static const char* contentLengthHeader = "Content-Length";
static const char* fileHeaderDivider = "\r\n\r\n";

const int fileBufSize = 4096;

static void restartTask(void* parameter) {
    vTaskDelay(pdMS_TO_TICKS(5000)); // Delay for 5 seconds
    esp_restart(); // Restart the ESP32
}

static void startRestartTask() {
    xTaskCreate(
        restartTask, // Task function
        "r", // Task name
        2048, // Stack size
        NULL, // Task input parameter
        1, // Priority
        NULL // Task handle
    );
};

static esp_err_t index_handler(httpd_req_t* req) {
    auto res = httpd_resp_sendstr(req, html);
    return res;
}

static int getContentLength(httpd_req_t* req) {
    char contentLength[64];
    auto contentLenLen = httpd_req_get_hdr_value_len(req, contentLengthHeader);

    auto err = httpd_req_get_hdr_value_str(req, contentLengthHeader, contentLength, sizeof(contentLength));
    if (err != ESP_OK) {
        return -1;
    }

    char* endptr;
    int contentLengthValue = strtol(contentLength, &endptr, 10);

    return contentLengthValue;
}

String getQueryPath(httpd_req_t* req, bool* sdCardSelected) {
    const int queryStringSize = 512;
    char queryString[queryStringSize];

    String result;

    auto err = httpd_req_get_url_query_str(req, queryString, queryStringSize);
    if (err == ESP_OK) {
        auto query = strnstr(queryString, "p=", queryStringSize);
        if (query != NULL) {
            query += 2;
            auto queryEnd = strchrnul(query, '&');
            result = String(query, queryEnd - query);
        }

        sdCardSelected[0] = strnstr(queryString, "sd=true", queryStringSize) != NULL;
    } else {
        sdCardSelected[0] = false;
    }

    return result;
}

static esp_err_t replyWithDirectory(httpd_req_t* req, DIR* dir, const String& query, bool sdCardSelected) {
    // directory listing
    lilka::serial.log("List dir %s", query);

    esp_err_t err = httpd_resp_set_type(req, "text/html; charset=UTF-8");
    if (err == ESP_OK) {
        const struct dirent* direntry = NULL;
        httpd_resp_sendstr_chunk(req, htmlListBegin);
        while ((direntry = readdir(dir)) != NULL) {
            String itemHtml("<li><a href='/download");
            auto absolutePath = lilka::fileutils.joinPath(query, direntry->d_name);

            if (sdCardSelected) {
                itemHtml += "?sd=true&p=";
            } else {
                itemHtml += "?p=";
            }

            itemHtml += absolutePath;

            if (direntry->d_type == DT_DIR) {
                itemHtml += "'>📁";
            } else {
                itemHtml += "'>📄";
            }
            itemHtml += direntry->d_name;
            itemHtml += "</a></li>";
            httpd_resp_sendstr_chunk(req, itemHtml.c_str());
        }
        httpd_resp_sendstr_chunk(req, htmlListPageEnd);
        httpd_resp_send_chunk(req, 0, 0);
    }

    return err;
}

static esp_err_t replyWithFile(httpd_req_t* req, const String& path) {
    esp_err_t err = ESP_OK;
    String fileNameHeader;

    auto file = fopen(path.c_str(), "r");
    if (file == NULL) {
        httpd_resp_send_404(req);
        return ESP_OK;
    }

    for (auto scanForName = path.end(); scanForName > path.begin(); scanForName--) {
        if (*scanForName == '/' || *scanForName == '\\') {
            scanForName++; // skip separator
            fileNameHeader = "attachment; filename=\"";
            fileNameHeader += scanForName;
            fileNameHeader += "\"";
            err = httpd_resp_set_hdr(req, "Content-Disposition", fileNameHeader.c_str());
            break;
        }
    }

    if (err == ESP_OK) {
        char* fileBuf = static_cast<char*>(malloc(fileBufSize));

        err = httpd_resp_set_type(req, "application/octet-stream");
        if (err == ESP_OK) {
            do {
                auto read = fread(fileBuf, 1, fileBufSize, file);
                err = httpd_resp_send_chunk(req, fileBuf, read);
                if (err != ESP_OK) {
                    break;
                }
            } while (read > 0);
        }

        free(fileBuf);
    }

    fclose(file);
    return err;
}

static esp_err_t download_handler(httpd_req_t* req) {
    bool sdCardSelected;
    esp_err_t err = ESP_OK;
    struct stat statbuf;
    statbuf.st_mode = _IFDIR;

    String query = getQueryPath(req, &sdCardSelected);
    auto root = sdCardSelected ? lilka::fileutils.getSDRoot() : lilka::fileutils.getSPIFFSRoot();
    auto path = lilka::fileutils.joinPath(root, query);

    if (stat(path.c_str(), &statbuf) != 0) {
        statbuf.st_mode = _IFDIR;
    }

    if (S_ISDIR(statbuf.st_mode)) {
        DIR* dir = opendir(path.c_str());
        if (dir != NULL) {
            err = replyWithDirectory(req, dir, query, sdCardSelected);
            closedir(dir);
        }
    } else {
        err = replyWithFile(req, path);
    }

    return err;
}

static esp_err_t upload_handler(httpd_req_t* req) {
    String lastError = "No error";
    esp_ota_handle_t ota_handle;
    esp_err_t res = httpd_resp_set_type(req, "text/html");

    if (res != ESP_OK) {
        lastError = "Failed to set response type";
        return res;
    }

    int contentLength = getContentLength(req);

    lilka::serial.log("FW upload binary size %d", contentLength);

    auto current_partition = esp_ota_get_running_partition();
    auto ota_partition = esp_ota_get_next_update_partition(current_partition);

    esp_err_t err = esp_ota_begin(ota_partition, contentLength, &ota_handle);

    if (err == ESP_OK) {
        char* buf = static_cast<char*>(malloc(fileBufSize));
        bool seekBinary = true;
        lilka::serial.log("FW upload begin");

        int len = 0;
        do {
            len = httpd_req_recv(req, buf, fileBufSize);
            if (len < 0) {
                lilka::serial.log("FW upload error %d", len);
                lastError = "Failed to receive firmware file";
                break;
            }

            char* towrite = buf;
            if (seekBinary) {
                auto beforeBinary = strnstr(buf, fileHeaderDivider, len);
                towrite = beforeBinary + 4;
                len -= towrite - buf;
                seekBinary = false;
            }

            if (len > 0) {
                err = esp_ota_write(ota_handle, towrite, len);
                if (err != ESP_OK) {
                    lastError = "Failed to write OTA partition";
                    break;
                }
            }
        } while (len > 0);

        if (err == ESP_OK) {
            err = esp_ota_end(ota_handle);
            if (err == ESP_OK) {
                err = esp_ota_set_boot_partition(ota_partition);
                if (err != ESP_OK) {
                    lastError = "Failed to set a new boot partition";
                }
            }
            else {
                lastError = "Failed to finalize OTA partition";
            }
        }

        free(buf);
    }
    else {
        lastError = "Failed to get ota partition";
    }

    if (err == ESP_OK) {
        // reply & restart
        res = httpd_resp_sendstr(req, html);
        lilka::serial.log("FW upload done. Restart scheduled");
        startRestartTask(); // deffered restart
    } else {
        lilka::serial.log("FW upload failed. Error %d", (int)err);
        // abort OTA
        esp_ota_abort(ota_handle);
        // reply with error info
        res = httpd_resp_set_status(req, HTTPD_500);
        lastError += fileHeaderDivider;
        res = httpd_resp_send(req, lastError.c_str(), lastError.length());
    }

    return res;
}

static void stopWebServer() {
    lilka::serial.log("Stopping web service");
    httpd_stop(stream_httpd);
    // ignore errors
}

static void startWebServer() {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = 80;

    httpd_uri_t index_uri = {.uri = "/", .method = HTTP_GET, .handler = index_handler, .user_ctx = NULL};
    httpd_uri_t upload_fw = {.uri = "/firmware", .method = HTTP_POST, .handler = upload_handler, .user_ctx = NULL};
    httpd_uri_t download_uri = {.uri = "/download", .method = HTTP_GET, .handler = download_handler, .user_ctx = NULL};

    lilka::serial.log("Start web service on %d", config.server_port);
    if (httpd_start(&stream_httpd, &config) == ESP_OK) {
        httpd_register_uri_handler(stream_httpd, &index_uri);
        httpd_register_uri_handler(stream_httpd, &upload_fw);
        httpd_register_uri_handler(stream_httpd, &download_uri);
    }
}

WebService::WebService() : Service("web") {
    networkService = ServiceManager::getInstance()->getService<NetworkService>("network");
}

WebService::~WebService() {
}

void WebService::run() {
    bool wasOnline = false;
    setStackSize(8192);

    while (true) {
        if (!networkService) {
            vTaskDelay(1000 / portTICK_PERIOD_MS);
            continue;
        }

        bool isOnline = networkService->getNetworkState() == NetworkState::NETWORK_STATE_ONLINE;

        if (getEnabled() && isOnline && !wasOnline) {
            startWebServer();
            wasOnline = true;
        } else if ((!getEnabled() || !isOnline) && wasOnline) {
            wasOnline = false;
            stopWebServer();
        }

        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}