#include "abstractwidget.h"

bool AbstractWidget::isFinished() {
    if (done) {
        done = false;
        return true;
    }
    return false;
}
void AbstractWidget::addActivationButton(lilka::Button activationButton) {
    if (std::find(activationButtons.begin(), activationButtons.end(), activationButton) != activationButtons.end())
        return;

    activationButtons.push_back(activationButton);
}
void AbstractWidget::update() {
    // we can find a caller through std, but it's heavy, so better not to
    lilka::serial.log("Update method not implemented yet");
}
void AbstractWidget::draw(Arduino_GFX* canvas) {
    lilka::serial.log("Draw method not implemented yet");
}

void AbstractWidget::removeActivationButton(lilka::Button activationButton) {
    activationButtons.erase(
        std::remove(activationButtons.begin(), activationButtons.end(), activationButton), activationButtons.end()
    );
}

lilka::Button AbstractWidget::getButton() {
    return button;
}
