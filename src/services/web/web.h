#pragma once

#include "keira/service.h"
#include "services/network/network.h"

class WebService : public Service {
public:
    WebService();
    ~WebService();

private:
    void run() override;
    NetworkService* networkService;
};