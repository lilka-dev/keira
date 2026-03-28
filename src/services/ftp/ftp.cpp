#include "ftp.h"

#include "keira/ksystem.h"
#include "keira/keira_lang.h"
#include "services/network/network.h"

REG_SERVICE(ftp, FTPService, false);
REG_CONFIG(ftp, password, KCONFIG_STRING, K_S_LAUNCHER_FTP_PASSWORD, "");

FTPService::~FTPService() {
    if (ftpServer) {
        delete ftpServer;
    }
    vSemaphoreDelete(ftpMtx);
}

void FTPService::onStart() {
    KeiraRegistryEntry* entry = ksystem.registry[getName()];
    setpassword(getConfig()["password"].s);
    if (getpassword().isEmpty()) {
        createPassword();
    }
}

void FTPService::run() {
    // Await network service

    while (networkService == NULL) {
        networkService = static_cast<NetworkService*>(ksystem.services["network"]);
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }

    ftpServer = new FtpServer();
    ftpServer->begin(getuser().c_str(), getpassword().c_str());

    while (true) {
        // TODO: Ensure no need FTP recreate on network state change
        ftpServer->handleFTP();
        taskYIELD();
    }
}

void FTPService::createPassword() {
    char pwd[FTP_PASSWORD_LENGTH + 1];
    for (int i = 0; i < FTP_PASSWORD_LENGTH; i++) {
        pwd[i] = random(0, 2) == 0 ? random(48, 57) : random(97, 122);
    }
    pwd[FTP_PASSWORD_LENGTH] = 0;

    auto cfg = getConfig();

    auto pwdEntry = cfg["password"];
    pwdEntry.s = pwd;
    cfg.set(pwdEntry);

    setpassword(String(pwd));

    if (ftpServer) {
        ftpServer->credentials(getuser().c_str(), getpassword().c_str());
    }
}
