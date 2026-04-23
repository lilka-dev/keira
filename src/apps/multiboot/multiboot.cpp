#include "multiboot.h"
#include "keira/keira.h"
#include "keira/utils/string.h"

MultiBootApp::MultiBootApp(const String& path) : App("MultiBoot") {
    this->firmwarePath = path;
    setktStackSize(8192); // Multiboot internally uses 4KB chunk
}

void MultiBootApp::run() {
    fileLoadAsRom(firmwarePath);
}

void MultiBootApp::fileLoadAsRom(const String& path) {
    // Draw Welcome message
    lilka::ProgressDialog dialog(K_S_FMANAGER_LOADING, path + "\n\n" K_S_FMANAGER_MULTIBOOT_STARTING);
    dialog.draw(canvas);
    queueDraw();

    // Trying to start upload
    int error;
    error = lilka::multiboot.start(path);
    if (error) {
        alert(K_S_ERROR, StringFormat(K_S_FMANAGER_MULTIBOOT_ERROR_FMT, 1, error));
        return;
    }
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