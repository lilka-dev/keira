#include "thread.h"
#include <FreeRTOS.h>
#include <lilka/serial.h>
//=============================================================================
//  Thread Constructors and destructors
//=============================================================================
KeiraThread::KeiraThread(
    KeiraCallback clbk, const char* ktName, uint32_t ktStackSize, KeiraCallbackData data, KeiraThreadPriority ktPrio,
    int ktCore
) {
    if (clbk) setupOnEntryCallback(clbk, data);
    if (ktName) setName(ktName);
    setStackSize(ktStackSize);
    setCore(ktCore);
    setPriority(ktPrio);

    KT_DBG lilka::serial.log(
        "Initialized thread %s with [clbk:%p, stack:%d, data:%p, ktPrio:%d, cpu:%d]",
        this->ktName,
        clbk,
        ktStackSize,
        data,
        ktPrio,
        ktCore
    );
}
//-----------------------------------------------------------------------------
KeiraThread::~KeiraThread() {
    stop();
}
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
//  Thread settings:
//=============================================================================
void KeiraThread::setPriority(KeiraThreadPriority ktPrio) {
    this->ktPrio = ktPrio;
    if (ktTaskHandle) vTaskPrioritySet(ktTaskHandle, ktPrio);
    KT_DBG lilka::serial.log("Changed ktPriority for task %s to %d", ktName, ktPrio);
}
//-----------------------------------------------------------------------------
void KeiraThread::setStackSize(uint32_t ktStackSize) {
    if (ktTaskHandle != NULL) {
        KT_DBG lilka::serial.err("Can't readjust ktStackSize for thread %s. Thread is already runing", ktName);
        return;
    }
    this->ktStackSize = ktStackSize;
}
//-----------------------------------------------------------------------------
void KeiraThread::setCore(int ktCore) {
    if ((ktCore != tskNO_AFFINITY) && (ktCore > portNUM_PROCESSORS || ktCore < 0)) {
        KT_DBG lilka::serial.err(
            "Can't pin thread %s to CPUCore %d. Max CPUCores is %d", ktName, ktCore, portNUM_PROCESSORS
        );
        return;
    }

    this->ktCore = ktCore;

    KT_DBG lilka::serial.log("Set core for thread %s to %d", getName(), ktCore);

    // TODO:    if (ktTaskHandle)  vTaskSetAffinity(ktCore);
}
//-----------------------------------------------------------------------------
KeiraThreadPriority KeiraThread::getPriority() {
    return ktPrio;
}
//-----------------------------------------------------------------------------
uint32_t KeiraThread::getStackSize() {
    return ktStackSize;
}
//-----------------------------------------------------------------------------
int KeiraThread::getCore() {
    return ktCore;
}
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
//  Thread stats/ktStates/information
//=============================================================================
const KeiraThreadState KeiraThread::getState() {
    if (ktTaskHandle == NULL) return KTS_INVALID;

    if (ktState == KTS_EXITING) return KTS_EXITING;

    ktState = static_cast<KeiraThreadState>(eTaskGetState(ktTaskHandle));

    return ktState;
}
//-----------------------------------------------------------------------------
const char* KeiraThread::getName() {
    return ktName;
}
//-----------------------------------------------------------------------------
void KeiraThread::setName(const char* ktName) {
    if (strlen(ktName) > KT_NAME_MAX - 1) {
        KT_DBG lilka::serial.err("Thread ktName shouldn't be longer than KT_MAX");
        return;
    }
    KT_DBG lilka::serial.log("Changing thread ktName from %s to %s", this->ktName, ktName);
    strcpy(this->ktName, ktName);
}
//-----------------------------------------------------------------------------
const KeiraThreadType KeiraThread::getType() {
    return ktType;
}
//-----------------------------------------------------------------------------
void KeiraThread::setType(KeiraThreadType ktType) {
    this->ktType = ktType;
}
//-----------------------------------------------------------------------------
TaskHandle_t KeiraThread::getTaskHandle() {
    return ktTaskHandle;
}
///////////////////////////////////////////////////////////////////////////////
#define LAUNCH_CALLBACKS(CLBK_TYPE)    \
    for (auto& callback : ktClbkTable) \
        if (callback.type == CLBK_TYPE) callback.clbk(callback.data);
// Our wrapper arround users run()
void KeiraThread::_run() {
    LAUNCH_CALLBACKS(KT_THREAD_CLBK);

    run();
    // Wait for being stopped by system manager
    exit();
}
//-----------------------------------------------------------------------------
// Special case, can be run from everywhere
void KeiraThread::exit() {
    LAUNCH_CALLBACKS(KT_ON_EXIT_CLBK);

    onExit();

    while (true) {
        vTaskDelay(portMAX_DELAY);
    }
    // Impossible  (^_^) == \~
    // vTaskDelete(NULL);
    KT_DBG lilka::serial.log("Exiting thread %s", ktName);
}
//=============================================================================
//  FreeRTOS task management to be done from system task
//=============================================================================
void KeiraThread::start() {
    if (ktTaskHandle != NULL) {
        KT_DBG lilka::serial.log("Thread %s already runing", ktName);
        return;
    }

    LAUNCH_CALLBACKS(KT_ON_START_CLBK);

    // Launch virtual method
    onStart();

    // Time to launch FreeRTOS task
    if (xTaskCreatePinnedToCore(
            reinterpret_cast<TaskFunction_t>(&KeiraThread::_run),
            ktName,
            ktStackSize,
            this,
            ktPrio,
            &ktTaskHandle,
            ktCore
        ) != pdPASS) {
        KT_DBG lilka::serial.err("Not enough heap to allocate stack for %s thread", ktName);
    }

    KT_DBG lilka::serial.log("Started thread %s", ktName);
}
//-----------------------------------------------------------------------------
void KeiraThread::suspend() {
    if (ktTaskHandle == NULL) {
        KT_DBG lilka::serial.err("Thread %s is not running, cannot suspend", ktName);
        return;
    }

    LAUNCH_CALLBACKS(KT_ON_SUSPEND_CLBK);

    // Launch overrided virtual method
    onSuspend();

    //    KT_DBG lilka::serial.log("Suspending thread %s (ktState = %s)", ktName, getState());

    vTaskSuspend(ktTaskHandle);
    KT_DBG lilka::serial.log("Suspended thread %s", ktName);
}
//-----------------------------------------------------------------------------
void KeiraThread::resume() {
    if (ktTaskHandle == NULL) {
        KT_DBG lilka::serial.err("Thread %s is not running, cannot resume", ktName);
        return;
    }

    LAUNCH_CALLBACKS(KT_ON_RESUME_CLBK);

    // Launch overrided virtual method
    onResume();

    vTaskResume(ktTaskHandle);

    KT_DBG lilka::serial.log("Resumed thread %s", ktName);
}
//-----------------------------------------------------------------------------
void KeiraThread::stop() {
    if (ktTaskHandle == NULL) {
        KT_DBG lilka::serial.err("Thread %s is not running, cannot stop", ktName);
        return;
    }

    LAUNCH_CALLBACKS(KT_ON_STOP_CLBK);

    // Launch overrided virtual method
    onStop();

    vTaskDelete(ktTaskHandle);
    KT_DBG lilka::serial.log("Deleted thread %s", ktName);

    ktTaskHandle = NULL;
}
#undef LAUNCH_CALLBACKS
///////////////////////////////////////////////////////////////////////////////

void KeiraThread::clearCallbacks() {
    ktClbkTable.clear();
}
