#pragma once

#include <lilka.h>
#include <AudioOutputI2S.h>

#include "keira/app.h"
#include "keira/ksound/sound.h"
#include "keira/ksound/analyzer.h"
#include "keira/ksound/audioplayer.h"

class MadPlayerApp : public App {
public:
    explicit MadPlayerApp(String path);
    void run() override;

private:
    void mainWindow();
    String fileName;

    lilka::Sound* sound = nullptr;
    AudioOutputI2S* i2sOutput = nullptr;
    lilka::AudioOutputAnalyzer* analyzer = nullptr;
};
