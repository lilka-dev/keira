#include "pastebinApp.h"
#include <HTTPClient.h>
#include <lilka/config.h>

pastebinApp::pastebinApp() : App("pastebin loader") {
}

void pastebinApp::run() {
    uiLoop();
}

void pastebinApp::uiLoop() {
    WiFiClientSecure client;
    HTTPClient http;
    String link_code = "";
    String filename = "";

    String path_pastebin_folder = "/pastebin";
    String pastebin_url = "https://pastebin.com/raw/";

    while (1) {
        lilka::Menu settingsMenu("Pastebin");
        settingsMenu.addActivationButton(K_BTN_BACK);
        settingsMenu.addItem(K_S_PASTEBIN_CODE, 0, lilka::colors::White, link_code);
        settingsMenu.addItem(K_S_PASTEBIN_NAME, 0, lilka::colors::White, filename);
        settingsMenu.addItem(K_S_PASTEBIN_DOWNLOAD, 0, lilka::colors::White, "");
        while (!settingsMenu.isFinished()) {
            settingsMenu.update();
            settingsMenu.draw(canvas);
            queueDraw();
        }
        if (settingsMenu.getButton() == K_BTN_BACK) {
            return; // exit
        } else {
            if (settingsMenu.getCursor() == 0) {
                lilka::InputDialog inputDialog(String(K_S_PASTEBIN_ENTER_CODE));
                inputDialog.setValue(link_code);
                while (!inputDialog.isFinished()) {
                    inputDialog.update();
                    inputDialog.draw(canvas);
                    queueDraw();
                }
                link_code = inputDialog.getValue();
            } else if (settingsMenu.getCursor() == 1) {
                lilka::InputDialog inputDialog(String(K_S_PASTEBIN_ENTER_NAME));
                inputDialog.setValue(filename);
                while (!inputDialog.isFinished()) {
                    inputDialog.update();
                    inputDialog.draw(canvas);
                    queueDraw();
                }
                filename = inputDialog.getValue();
            } else if (settingsMenu.getCursor() == 2) {
                if (filename.length() == 0) {
                    filename = link_code;
                    continue;
                }

                FRESULT res = f_stat(path_pastebin_folder.c_str(), nullptr);

                if (res == FR_NO_FILE) {
                    res = f_mkdir(path_pastebin_folder.c_str());
                    if (res != FR_OK) {
                        alert("pastebin", K_S_PASTEBIN_ERROR_CREATE_DIR);
                    }
                } else if (res != FR_OK) {
                    alert("pastebin", K_S_PASTEBIN_ERROR_CREATE_DIR);
                } else {
                    String url = pastebin_url + link_code;
                    String fullPath = path_pastebin_folder + "/" + filename;

                    client.setInsecure();
                    http.begin(client, url);
                    int httpCode = http.GET();

                    if (httpCode == HTTP_CODE_OK) {
                        String dlFilename = lilka::fileutils.joinPath(LILKA_SD_ROOT, fullPath);
                        // Open file for writing
                        FILE* file = fopen(dlFilename.c_str(), FILE_WRITE);
                        if (!file) {
                            alert("pastebin", K_S_PASTEBIN_ERROR_OPEN_FILE);
                            break;
                        }

                        fprintf(file, "%s", http.getString().c_str());
                        fclose(file);

                        delay(10);

                        alert("pastebin", StringFormat(K_S_PASTEBIN_FILE_DOWNLOADED_FMT, dlFilename.c_str()));

                        break;
                    } else {
                        alert("pastebin", StringFormat(K_S_PASTEBIN_REQUEST_FAIL_FMT, http.errorToString(httpCode)));
                    }
                }
            }
        }
    }
}
