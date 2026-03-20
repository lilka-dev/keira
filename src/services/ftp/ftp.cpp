#include "ftp.h"

// Libraries
#include <Preferences.h>

REG_SERVICE("ftp", FTPService, false);

#include "keira/ksystem.h"
#include "services/network/network.h"

FTPService::FTPService() {
    NVS_LOCK;
    Preferences prefs;
    prefs.begin(getName(), true);
    password = prefs.getString("password", "");
    prefs.end();
    NVS_UNLOCK;

    if (password.isEmpty()) {
        createPassword();
    }

    lilka::fileutils.initSD();
}

FTPService::~FTPService() {
    if (ftpServer) {
        delete ftpServer;
    }
}

void FTPService::run() {
    // Await network service
    /*
    while (networkService == NULL) {
        networkService = static_cast<NetworkService*>(ksystem.services["network"]);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    */
    ftpServer = new FtpServer();
    ftpServer->begin(user.c_str(), password.c_str());

    while (true) {
        // TODO: Ensure no need FTP recreate on network state change
        ftpServer->handleFTP();
        taskYIELD();
    }
}

String FTPService::getUser() {
    return user;
}

String FTPService::getPassword() {
    return password;
}

void FTPService::createPassword() {
    char pwd[FTP_PASSWORD_LENGTH + 1];
    for (int i = 0; i < FTP_PASSWORD_LENGTH; i++) {
        pwd[i] = random(0, 2) == 0 ? random(48, 57) : random(97, 122);
    }
    pwd[FTP_PASSWORD_LENGTH] = 0;
    NVS_LOCK;
    Preferences prefs;
    prefs.begin(getName(), false);
    prefs.putString("password", pwd);
    prefs.end();
    NVS_UNLOCK;

    password = String(pwd);

    if (ftpServer) {
        ftpServer->credentials(user.c_str(), password.c_str());
    }
}
