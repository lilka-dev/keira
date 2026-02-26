#include "thread.h"
#include <FreeRTOS.h>
#include <lilka/serial.h>
//=============================================================================
//  Thread Constructors and destructors
//=============================================================================
KeiraThread::KeiraThread(
    KeiraCallback clbk, const char* name, uint32_t stackSize, KeiraCallbackData data, KeiraThreadPriority prio,
    int cpuCore
) {
    if (clbk) setupOnEntryCallback(clbk, data);
    setName(name);
    setStackSize(stackSize);
    setCore(cpuCore);
    setPriority(prio);
}
//-----------------------------------------------------------------------------
KeiraThread::~KeiraThread() {
    stop();
}
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
//  Thread settings:
//=============================================================================
void KeiraThread::setPriority(KeiraThreadPriority prio) {
    this->prio = prio;
    if (taskHandle) vTaskPrioritySet(taskHandle, prio);
}
//-----------------------------------------------------------------------------
void KeiraThread::setStackSize(uint32_t stackSize) {
    if (taskHandle != NULL) {
        KT_DBG lilka::serial.err("Can't readjust stackSize for thread %s. Thread is already runing", name);
        return;
    }
    this->stackSize = stackSize;
}
//-----------------------------------------------------------------------------
void KeiraThread::setCore(int core) {
    if ((core != tskNO_AFFINITY) && (core > portNUM_PROCESSORS || core < 0)) {
        KT_DBG lilka::serial.err(
            "Can't pin thread %s to CPUCore %d. Max CPUCores is %d", name, core, portNUM_PROCESSORS
        );
        return;
    }

    this->core = core;

    // TODO:    if (taskHandle)  vTaskSetAffinity(core);
}
//-----------------------------------------------------------------------------
KeiraThreadPriority KeiraThread::getPriority() {
    return prio;
}
//-----------------------------------------------------------------------------
uint32_t KeiraThread::getStackSize() {
    return stackSize;
}
//-----------------------------------------------------------------------------
int KeiraThread::getCore() {
    return core;
}
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
//  Thread stats/states/information
//=============================================================================
const KeiraThreadState KeiraThread::getState() {
    if (taskHandle == NULL) return KTS_INVALID;

    if (state == KTS_EXITING) return KTS_EXITING;

    state = static_cast<KeiraThreadState>(eTaskGetState(taskHandle));

    return state;
}
//-----------------------------------------------------------------------------
const char* KeiraThread::getName() {
    return name;
}
//-----------------------------------------------------------------------------
void KeiraThread::setName(const char* name) {
    if (strlen(name) > KT_NAME_MAX - 1) {
        KT_DBG lilka::serial.err("Thread name shouldn't be longer than KT_MAX");
    } else strcpy(this->name, name);
}
//-----------------------------------------------------------------------------
const KeiraThreadType KeiraThread::getType() {
    return type;
}
//-----------------------------------------------------------------------------
void KeiraThread::setType(KeiraThreadType type) {
    this->type = type;
}
//-----------------------------------------------------------------------------
TaskHandle_t KeiraThread::getTaskHandle() {
    return taskHandle;
}
///////////////////////////////////////////////////////////////////////////////
#define LAUNCH_CALLBACKS(CLBK_TYPE)  \
    for (auto& callback : clbkTable) \
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
}
//=============================================================================
//  FreeRTOS task management to be done from system task
//=============================================================================
void KeiraThread::start() {
    if (taskHandle != NULL) {
        KT_DBG lilka::serial.log("Thread %s already runing", name);
        return;
    }

    LAUNCH_CALLBACKS(KT_ON_START_CLBK);

    // Launch virtual method
    onStart();

    // Time to launch FreeRTOS task
    if (xTaskCreatePinnedToCore(
            reinterpret_cast<TaskFunction_t>(&KeiraThread::_run), name, stackSize, this, prio, &taskHandle, core
        ) != pdPASS) {
        KT_DBG lilka::serial.err("Not enough heap to allocate stack for %s thread", name);
    }
}
//-----------------------------------------------------------------------------
void KeiraThread::suspend() {
    if (taskHandle == NULL) {
        KT_DBG lilka::serial.err("Thread %s is not running, cannot suspend", name);
        return;
    }

    LAUNCH_CALLBACKS(KT_ON_SUSPEND_CLBK);

    // Launch overrided virtual method
    onSuspend();

    //    KT_DBG lilka::serial.log("Suspending thread %s (state = %s)", name, getState());

    vTaskSuspend(taskHandle);
}
//-----------------------------------------------------------------------------
void KeiraThread::resume() {
    if (taskHandle == NULL) {
        KT_DBG lilka::serial.err("Thread %s is not running, cannot resume", name);
        return;
    }

    LAUNCH_CALLBACKS(KT_ON_RESUME_CLBK);

    // Launch overrided virtual method
    onResume();

    vTaskResume(taskHandle);
}
//-----------------------------------------------------------------------------
void KeiraThread::stop() {
    if (taskHandle == NULL) {
        KT_DBG lilka::serial.err("Thread %s is not running, cannot stop", name);
        return;
    }

    LAUNCH_CALLBACKS(KT_ON_STOP_CLBK);

    // Launch overrided virtual method
    onStop();

    vTaskDelete(taskHandle);

    taskHandle = NULL;
}
#undef LAUNCH_CALLBACKS
///////////////////////////////////////////////////////////////////////////////

void KeiraThread::clearCallbacks() {
    clbkTable.clear();
}
