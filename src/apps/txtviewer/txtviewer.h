#pragma once
#include "app.h"
#include "txtview.h"

class TxtViewerApp : public App {
public:
    explicit TxtViewerApp(String fPath);
    TxtView tView;

private:
    TxtViewerApp();
    void run() override;
};
