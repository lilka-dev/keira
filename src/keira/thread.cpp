#include "thread.h"
#include <FreeRTOS.h>
#include <lilka/serial.h>
#include "keira/mutex.h"

#ifdef KEIRA_WATCHDOG
#    include "services/watchdog/watchdog.h"
#    include "keira/ksystem.h"
#endif
//=============================================================================
//  Thread Constructors and destructors
//=============================================================================
KeiraThread::KeiraThread(
    KeiraCallback clbk, const char* ktName, uint32_t ktStackSize, KeiraCallbackData data, KeiraThreadPriority ktPriority,
    int ktCore
) {
    if (clbk) setupOnEntryCallback(clbk, data);

    if (ktName) setName(ktName);
    else setName(KT_DEFAULT_NAME);

    setktStackSize(ktStackSize);
    setktCore(ktCore);
    setktPriority(ktPriority);
}
//-----------------------------------------------------------------------------
KeiraThread::~KeiraThread() {
    // Calling stop() here seems to be undefined behavior
    KMTX_LOCK(ktLock);

    if (ktTaskHandle) {
        vTaskDelete(ktTaskHandle);
        ktTaskHandle = NULL;

    } else {
        KT_DBG lilka::serial.err("Trying to stop non-existing task from Thread destructor");
    }

    KMTX_UNLOCK(ktLock);
}
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
//  Thread stats/ktStates/information
//=============================================================================
const KeiraThreadState KeiraThread::getState() {
    KMTX_LOCK(ktLock);

    if (ktTaskHandle == NULL) {
        KMTX_UNLOCK(ktLock);

        return KTS_INVALID;
    }

    if (ktState == KTS_EXITING) {
        KMTX_UNLOCK(ktLock);

        return KTS_EXITING;
    }

    ktState = static_cast<KeiraThreadState>(eTaskGetState(ktTaskHandle));

    auto tmpState = ktState;

    KMTX_LOCK(ktLock);

    return tmpState;
}
//-----------------------------------------------------------------------------
// pass via ptr
const char* KeiraThread::getName() {
    return ktName;
}
//-----------------------------------------------------------------------------
void KeiraThread::setName(const char* ktName) {
    KMTX_LOCK(ktLock);

    if (strlen(ktName) > KT_NAME_MAX - 1) {
        KT_DBG lilka::serial.err("Thread ktName shouldn't be longer than KT_NAME_MAX");
        KMTX_UNLOCK(ktLock);
        return;
    }
    KT_DBG lilka::serial.log("Changing thread ktName from %s to %s", this->ktName, ktName);

    strcpy(this->ktName, ktName);

    KMTX_UNLOCK(ktLock);
}

///////////////////////////////////////////////////////////////////////////////
#define LAUNCH_CALLBACKS(CLBK_TYPE)                                   \
    KMTX_LOCK(ktLock);                                                \
    for (auto& callback : ktClbkTable)                                \
        if (callback.type == CLBK_TYPE) callback.clbk(callback.data); \
    KMTX_UNLOCK(ktLock);
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

    KMTX_LOCK(ktLock);

    ktState = KTS_EXITING;

    KT_DBG lilka::serial.log("Exiting thread %s", ktName);

    KMTX_UNLOCK(ktLock);

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
    KMTX_LOCK(ktLock);

    if (ktTaskHandle != NULL) {
        KT_DBG lilka::serial.log("Thread %s already runing", ktName);
        KMTX_UNLOCK(ktLock);
        return;
    }

    KMTX_UNLOCK(ktLock);

    LAUNCH_CALLBACKS(KT_ON_START_CLBK);

    // Launch virtual method
    onStart();

    KMTX_LOCK(ktLock);

    // Time to launch FreeRTOS task
    if (xTaskCreatePinnedToCore(
            reinterpret_cast<TaskFunction_t>(&KeiraThread::_run),
            ktName,
            ktStackSize,
            this,
            ktPriority,
            &ktTaskHandle,
            ktCore
        ) != pdPASS) {
        KT_DBG lilka::serial.err("Not enough heap to allocate stack for %s thread", ktName);
    }

#ifdef KEIRA_WATCHDOG
    WatchdogService* watchdog = reinterpret_cast<WatchdogService*>(ksystem.services["watchdog"]);
    watchdog->addTask(&ktTaskHandle, WATCHDOG_TASK_MISC);
#endif

    KT_DBG lilka::serial.log("Started thread %s", ktName);

    KMTX_UNLOCK(ktLock);
}
//-----------------------------------------------------------------------------
void KeiraThread::suspend() {
    KMTX_LOCK(ktLock);

    if (ktTaskHandle == NULL) {
        KT_DBG lilka::serial.err("Thread %s is not running, cannot suspend", ktName);
        KMTX_UNLOCK(ktLock);
        return;
    }

    if (ktState == KTS_EXITING) {
        KT_DBG lilka::serial.err("Trying to suspend exiting thread %s", ktName);
        KMTX_UNLOCK(ktLock);
        return;
    }

    KMTX_UNLOCK(ktLock);

    LAUNCH_CALLBACKS(KT_ON_SUSPEND_CLBK);

    // Launch overrided virtual method
    onSuspend();

    KMTX_LOCK(ktLock);

    //    KT_DBG lilka::serial.log("Suspending thread %s (ktState = %s)", ktName, getState());

    vTaskSuspend(ktTaskHandle);

    KT_DBG lilka::serial.log("Suspended thread %s", ktName);

    KMTX_UNLOCK(ktLock);
}
//-----------------------------------------------------------------------------
void KeiraThread::resume() {
    KMTX_LOCK(ktLock);

    if (ktTaskHandle == NULL) {
        KT_DBG lilka::serial.err("Thread %s is not running, cannot resume", ktName);
        KMTX_UNLOCK(ktLock);
        return;
    }

    if (ktState == KTS_EXITING) {
        KT_DBG lilka::serial.err("Trying to resume exiting thread %s", ktName);
        KMTX_UNLOCK(ktLock);
        return;
    }

    KMTX_UNLOCK(ktLock);

    LAUNCH_CALLBACKS(KT_ON_RESUME_CLBK);

    // Launch overrided virtual method
    onResume();

    KMTX_LOCK(ktLock);

    vTaskResume(ktTaskHandle);

    KT_DBG lilka::serial.log("Resumed thread %s", ktName);

    KMTX_UNLOCK(ktLock);
}
//-----------------------------------------------------------------------------
void KeiraThread::stop() {
    KMTX_LOCK(ktLock);

    if (ktTaskHandle == NULL) {
        KT_DBG lilka::serial.err("Thread %s is not running, cannot stop", ktName);
        KMTX_UNLOCK(ktLock);
        return;
    }

    KMTX_UNLOCK(ktLock);

    LAUNCH_CALLBACKS(KT_ON_STOP_CLBK);

    // Launch overrided virtual method
    onStop();

    KMTX_LOCK(ktLock);

    vTaskDelete(ktTaskHandle);

    ktTaskHandle = NULL;

    KT_DBG lilka::serial.log("Deleted thread %s", ktName);

    KMTX_UNLOCK(ktLock);
}
#undef LAUNCH_CALLBACKS
///////////////////////////////////////////////////////////////////////////////

void KeiraThread::clearCallbacks() {
    KMTX_LOCK(ktLock);

    ktClbkTable.clear();

    KMTX_LOCK(ktLock);
}
