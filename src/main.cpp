#include "keira/ksystem.h"
#include <FreeRTOS.h>

void setup() {
    ksystem.setup();
}

void loop() {
    vTaskDelete(NULL);
}
