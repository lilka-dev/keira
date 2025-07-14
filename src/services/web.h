#pragma once

#include "service.h"

class WebService : public Service {
public:
    WebService();
    ~WebService();

private:
    void run() override;
};