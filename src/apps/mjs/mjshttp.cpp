#include "mjshttp.h"
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <SD.h>
#include <FS.h>
#include "mjs.h"

#define STR_HELPER(x) #x
#define STR(x)        STR_HELPER(x)
static const char* defaultUserAgent = "Lilka/" STR(LILKA_VERSION);

// http.execute({url, method, body, file}) -> {code, response}
static void mjs_http_execute(struct mjs* mjs) {
    mjs_val_t opts = mjs_arg(mjs, 0);

    const char* url = NULL;
    const char* method = NULL;
    const char* body = NULL;
    const char* fileName = NULL;

    mjs_val_t url_val = mjs_get(mjs, opts, "url", ~0);
    if (mjs_is_string(url_val)) {
        url = mjs_get_cstring(mjs, &url_val);
    }
    mjs_val_t method_val = mjs_get(mjs, opts, "method", ~0);
    if (mjs_is_string(method_val)) {
        method = mjs_get_cstring(mjs, &method_val);
    }
    mjs_val_t body_val = mjs_get(mjs, opts, "body", ~0);
    if (mjs_is_string(body_val)) {
        body = mjs_get_cstring(mjs, &body_val);
    }
    mjs_val_t file_val = mjs_get(mjs, opts, "file", ~0);
    if (mjs_is_string(file_val)) {
        fileName = mjs_get_cstring(mjs, &file_val);
    }

    if (method == NULL) {
        method = (body == NULL) ? "GET" : "POST";
    }

    WiFiClientSecure client;
    client.setInsecure();

    HTTPClient http;
    http.setUserAgent(defaultUserAgent);
    http.begin(client, url);

    int statusCode;
    if (body == NULL) {
        statusCode = http.sendRequest(method);
    } else {
        statusCode = http.sendRequest(method, String(body));
    }

    mjs_val_t result = mjs_mk_object(mjs);
    mjs_set(mjs, result, "code", ~0, mjs_mk_number(mjs, statusCode));

    if (statusCode == HTTP_CODE_OK) {
        if (fileName != NULL) {
            WiFiClient* stream = http.getStreamPtr();
            File file = SD.open(fileName, FILE_WRITE, true);
            if (file) {
                uint8_t buffer[128];
                while (stream->connected() && stream->available()) {
                    size_t bytes = stream->readBytes(buffer, sizeof(buffer));
                    if (bytes > 0) {
                        file.write(buffer, bytes);
                    }
                }
                file.close();
            }
        } else {
            String response = http.getString();
            mjs_set(mjs, result, "response", ~0, mjs_mk_string(mjs, response.c_str(), ~0, 1));
        }
    }

    http.end();
    mjs_return(mjs, result);
}

void mjs_http_register(struct mjs* mjs) {
    mjs_val_t httpObj = mjs_mk_object(mjs);
    mjs_set(mjs, httpObj, "execute", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_http_execute));
    mjs_set(mjs, httpObj, "HTTP_CODE_OK", ~0, mjs_mk_number(mjs, HTTP_CODE_OK));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "http", ~0, httpObj);
}
