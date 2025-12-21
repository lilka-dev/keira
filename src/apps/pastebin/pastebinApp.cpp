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
        settingsMenu.addItem("Код", 0, lilka::colors::White, link_code);
        settingsMenu.addItem("Назва", 0, lilka::colors::White, filename);
        settingsMenu.addItem("Завантажити", 0, lilka::colors::White, "");
        while (!settingsMenu.isFinished()) {
            settingsMenu.update();
            settingsMenu.draw(canvas);
            queueDraw();
        }
        if (settingsMenu.getButton() == K_BTN_BACK) {
            return; // exit
        } else {
            if (settingsMenu.getCursor() == 0) {
                lilka::InputDialog inputDialog(String("Введіть код"));
                inputDialog.setValue(link_code);
                while (!inputDialog.isFinished()) {
                    inputDialog.update();
                    inputDialog.draw(canvas);
                    queueDraw();
                }
                link_code = inputDialog.getValue();
            } else if (settingsMenu.getCursor() == 1) {
                lilka::InputDialog inputDialog(String("Введіть назву"));
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
                        alert("pastebin", "Помилка створення директорії");
                        printf("Помилка створення директорії %d\n", res);
                    }
                } else if (res != FR_OK) {
                    alert("pastebin", "Помилка створення директорії");
                } else {
                    String url = pastebin_url + link_code;
                    String fullPath = path_pastebin_folder + "/" + filename;

                    client.setInsecure();
                    http.begin(client, url);
                    int httpCode = http.GET();

                    if (httpCode == HTTP_CODE_OK) {
                        // Open file for writing
                        FILE* file = fopen((lilka::fileutils.getSDRoot() + fullPath).c_str(), FILE_WRITE);
                        if (!file) {
                            alert("pastebin", "Помилка відкривання файлу");
                            printf("Помилка відкривання файлу");
                            break;
                        }

                        fprintf(file, "%s", http.getString().c_str());
                        fclose(file);

                        delay(10);

                        alert("pastebin", "Файл завантажено, та збережено");

                        printf("Файл завантажено, та збережено");
                        break;
                    } else {
                        alert("pastebin", "HTTP GET failed, error: " + http.errorToString(httpCode));

                        printf("HTTP GET failed, error: %s\n", http.errorToString(httpCode).c_str());
                    }
                }
            }
        }
    }
}
