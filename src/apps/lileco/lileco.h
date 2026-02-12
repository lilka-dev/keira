/** ColecoVision Emulator for Lilka - Main app: declarations ********/
/**                                                                **/
/** Author: Oleksii "Alder" Derkach  [https://github.com/alder/]   **/
/**                                                                **/
/********************************************************************/
#pragma once

#include "keira/app.h"
#include "coleco_core.h"

class LilecoApp : public App {
public:
    explicit LilecoApp(String path);
    void run() override;
    void drawHelpHint();

private:
    void showControlsOverlay(lileco::ColecoCore& core);

    String selectedFileName;
    String romPath;
};
