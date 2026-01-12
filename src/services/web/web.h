#pragma once

#include "keira/service.h"
#include "services/network/network.h"

#define WEB_BUFFER_FS_OP    4096
#define WEB_BUFFER_FLASH_OP 4096

class WebService : public Service {
public:
    WebService();
    ~WebService();

private:
    void run() override;
    NetworkService* networkService;
};