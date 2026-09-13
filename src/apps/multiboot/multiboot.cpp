#include "multiboot.h"
#include "keira/keira.h"
#include "keira/utils/string.h"

#define USE_SPIFFS_BACKUP 1

MultiBootApp::MultiBootApp(const String& path) : App("MultiBoot") {
    this->firmwarePath = path;
    setktStackSize(8192); // Multiboot internally uses 4KB chunk
}

void MultiBootApp::run() {
    if (firmwarePath == "") lilka::multiboot.bootLast();
    else fileLoadAsRom(firmwarePath);
}

void MultiBootApp::fileLoadAsRom(const String& path) {
    /* тут цікаве:
        - якщо той самий файл вже завантажено в ОТА-розділ, то просто запускаємо
        - якщо була завантажена інша прошивка, то бекапим її SPIFFS
        - якщо присутній бекап прошивки, що буде вантажитись - розгортаємо його в SPIFFS
        - розгортаємо BIN в OTA-розділ
    */
    // 


    // Draw Welcome message
    lilka::ProgressDialog dialog(K_S_FMANAGER_LOADING, path + "\n\n" K_S_FMANAGER_MULTIBOOT_STARTING);
    dialog.draw(canvas);
    queueDraw();

    int error;

    String lastOTA = lilka::multiboot.lastOTAFirmware();
    if (firmwarePath == lastOTA) {
        lilka::multiboot.bootLast();
        return;
    }

#ifdef USE_SPIFFS_BACKUP


    if (!lastOTA.isEmpty()) {
        // Треба бекап
        String spiffsPath = lastOTA + ".spfs";
        error = lilka::multiboot.startSPIFFSBackup(spiffsPath);

        if (error) {
            alert(K_S_ERROR, StringFormat(K_S_FMANAGER_MULTIBOOT_ERROR_FMT, 2, error));
            return;
        }

        dialog.setTitle("backing up");
        dialog.setMessage(StringFormat(
            K_S_FMANAGER_MULTIBOOT_ABOUT_FMT,
            spiffsPath.c_str(),
            lilka::fileutils.getHumanFriendlySize(lilka::multiboot.getBytesTotal()).c_str()
        ));
        dialog.draw(canvas);
        queueDraw();

        while ((error = lilka::multiboot.processBackup()) > 0) {
            int progress = lilka::multiboot.getBytesWritten() * 100 / lilka::multiboot.getBytesTotal();
            dialog.setProgress(progress);
            dialog.draw(canvas);
            queueDraw();
            if (lilka::controller.getState().a.justPressed) {
                lilka::multiboot.cancel();
                return;
            }
        }
    }

    String restorePath = firmwarePath + ".spfs";

    if (access(restorePath.c_str(), F_OK) == 0) {
        error = lilka::multiboot.startSPIFFSRestore(restorePath);

        if (error) {
            alert(K_S_ERROR, StringFormat(K_S_FMANAGER_MULTIBOOT_ERROR_FMT, 3, error));
            return;
        }

        dialog.setTitle("restoring...");
        dialog.setMessage(StringFormat(
            K_S_FMANAGER_MULTIBOOT_ABOUT_FMT,
            restorePath.c_str(),
            lilka::fileutils.getHumanFriendlySize(lilka::multiboot.getBytesTotal()).c_str()
        ));
        dialog.draw(canvas);
        queueDraw();

        while ((error = lilka::multiboot.processRestore()) > 0) {
            int progress = lilka::multiboot.getBytesWritten() * 100 / lilka::multiboot.getBytesTotal();
            dialog.setProgress(progress);
            dialog.draw(canvas);
            queueDraw();
            if (lilka::controller.getState().a.justPressed) {
                lilka::multiboot.cancel();
                return;
            }
        }
    }

#endif    

    // Trying to start upload
    error = lilka::multiboot.start(path);
    if (error) {
        alert(K_S_ERROR, StringFormat(K_S_FMANAGER_MULTIBOOT_ERROR_FMT, 1, error));
        return;
    }

    dialog.setTitle(K_S_FMANAGER_LOADING);
    dialog.setMessage(StringFormat(
        K_S_FMANAGER_MULTIBOOT_ABOUT_FMT,
        path.c_str(),
        lilka::fileutils.getHumanFriendlySize(lilka::multiboot.getBytesTotal()).c_str()
    ));
    dialog.draw(canvas);
    queueDraw();

    while ((error = lilka::multiboot.process()) > 0) {
        int progress = lilka::multiboot.getBytesWritten() * 100 / lilka::multiboot.getBytesTotal();
        dialog.setProgress(progress);
        dialog.draw(canvas);
        queueDraw();
        if (lilka::controller.getState().a.justPressed) {
            lilka::multiboot.cancel();
            return;
        }
    }
    if (error < 0) {
        alert(K_S_ERROR, StringFormat(K_S_FMANAGER_MULTIBOOT_ERROR_FMT, 2, error));
        return;
    }
    error = lilka::multiboot.finishAndReboot();
    if (error) {
        alert(K_S_ERROR, StringFormat(K_S_FMANAGER_MULTIBOOT_ERROR_FMT, 3, error));
        return;
    }
}