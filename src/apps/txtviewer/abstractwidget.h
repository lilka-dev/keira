#pragma once
#include <lilka.h>
// I feel like finally done repeating same stuff
// TODO: to be moved in sdk
// TODO: make the docs
class AbstractWidget {
public:
    bool isFinished();
    void addActivationButton(lilka::Button activationButton);
    void removeActivationButton(lilka::Button activationButton);
    lilka::Button getButton();
    // each widget have to provide own way to deal with that
    virtual void update();
    virtual void draw(Arduino_GFX* canvas);

protected:
    std::vector<lilka::Button> activationButtons;
    lilka::Button button;
    bool done = false;
};
