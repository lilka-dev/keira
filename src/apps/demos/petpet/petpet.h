#pragma once
#include <AnimatedGIF.h>
#include "keira/app.h"
class PetPetApp : public App {
public:
    PetPetApp();
    static void GIFDraw(GIFDRAW* pDraw);

private:
    void run() override;
};
