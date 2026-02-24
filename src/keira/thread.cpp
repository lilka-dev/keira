#include "thread.h"
#include <FreeRTOS.h>

#define LAUNCH_CALLBACKS(CLBK_TYPE)  \
    for (auto& callback : clbkTable) \
        if (callback.type == CLBK_TYPE) callback.clbk(callback.data);

// Run in a separate thread,
void KeiraThread::_run() {
    LAUNCH_CALLBACKS(KT_THREAD_CLBK);

    run();
    // Wait for being stopped by system task
    exit();
}

void KeiraThread::exit() {
    LAUNCH_CALLBACKS(KT_ON_EXIT_CLBK);

    onExit();

    while (true) {
        vTaskDelay();
    }
}

// FreeRTOS task operations
void KeiraThread::start() {
    LAUNCH_CALLBACKS(KT_ON_START_CLBK);

    // Launch virtual method
    onStart();
    // Actual task management
}
void KeiraThread::suspend() {
    LAUNCH_CALLBACKS(KT_ON_SUSPEND_CLBK);

    // Launch overrided virtual method
    onSuspend();
    // Actual task management
}
void KeiraThread::resume() {
    LAUNCH_CALLBACKS(KT_ON_RESUME_CLBK);

    // Launch overrided virtual method
    onResume();
    // Actual task management
}
void KeiraThread::stop() {
    LAUNCH_CALLBACKS(KT_ON_STOP_CLBK);

    // Launch overrided virtual method
    onStop();
    // Actual task management
}
#undef LAUNCH_CALLBACKS

void KeiraThread::clearCallbacks() {
    clbkTable.clear();
}
