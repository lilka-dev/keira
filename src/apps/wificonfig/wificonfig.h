#pragma once
#include "keira/keira.h"
#include "keira/app.h"

class WiFiConfigApp : public App {
public:
    WiFiConfigApp();

private:
    String getEncryptionTypeStr(uint8_t encryptionType);
    void run() override;
};
