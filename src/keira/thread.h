#pragma once
//////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
//////////////////////////////////////////////////////////////////////////////
// Unified class for all FreeRTOS Tasks
/////////////////////////////////////////////////////////////////////////////
// Purpose of this component to unify all FreeRTOS task management in
// KeiraOS
//
// Component supports overriding via Nested API or as a building block
// itself via Callback API or both at once
//
// Callback API supports specifying as well as infinity callbacks of any type
// whose would be launched in FIFO order
/////////////////////////////////////////////////////////////////////////////

//============================================================================
//  Keira Thread lifecycle:
//============================================================================
// Start -> Entry -> ( [ Suspend/Resume/Working ] * n ) -> [Stop/Exit]
//////////////////////////////////////////////////////////////////////////////

// Uncomment this line to get debugging information
// #define KEIRA_THREAD_DEBUG
#include "keira/bits/thread.h"
#include "keira/mutex.h"

// TODO: move to keira/bits/thread
// TODO: check max RTOS thread name length
#define KT_NAME_MAX 30

// Libraries:
#include <FreeRTOS.h>
#include <vector>
#include <stdint.h>

//============================================================================
//  Keira Thread casts
//============================================================================
#define KT_CLBK_CAST(X)      reinterpret_cast<KeiraCallback>(X)
#define KT_CLBK_DATA_CAST(X) reinterpret_cast<KeiraCallbackData>(X)
//////////////////////////////////////////////////////////////////////////////

class ThreadManager;
class AppManager;
class ServiceManager;
class KeiraSystem;

//============================================================================
//  Keira Thread types
//============================================================================
// To be set in derived classes (Nested API)
// Specifies which system component would control current thread
// management (a.k.a. system manager)
//============================================================================
typedef enum {
    KT_THREAD,
    KT_APP,
    KT_SERVICE,
    // KT_CMD_APP Maybe sometime in future
} KeiraThreadType;
//////////////////////////////////////////////////////////////////////////////

//============================================================================
//  Keira Thread states
//============================================================================
typedef enum {
    KTS_RUNING = eRunning,
    KTS_READY = eReady,
    KTS_BLOCKED = eBlocked,
    KTS_SUSPENDED = eSuspended,
    KTS_DELETED = eDeleted,
    KTS_INVALID = eInvalid,
    KTS_EXITING
} KeiraThreadState; // FreeRTOS eTaskState + one additional state
//////////////////////////////////////////////////////////////////////////////

//============================================================================
//  Keira Thread priorities
//============================================================================
typedef enum { KT_PRIO_IDLE, KT_PRIO_DEFAULT, KT_PRIO_MAX } KeiraThreadPriority;
// Note: FreeRTOS suggest as low amount of priorities as possible
// TODO: portPRIVELEGE_BIT ?
//////////////////////////////////////////////////////////////////////////////

// Thread name type
typedef const char* threadNameType_t;

class KeiraThread {
    friend ThreadManager;
    friend AppManager;
    friend ServiceManager;
    friend KeiraSystem;

public:
    //=======================================================================
    //  Thread Constructors and destructors
    //=======================================================================
    explicit KeiraThread(
        KeiraCallback clbk = NULL, const char* ktName = NULL, uint32_t ktStackSize = KT__STACK,
        KeiraCallbackData data = NULL, KeiraThreadPriority ktPriority = KT_PRIO_DEFAULT, int ktCore = KT__CORE
    );
    virtual ~KeiraThread();
    //////////////////////////////////////////////////////////////////////////
    //=======================================================================
    //  Thread settings
    //=======================================================================
    // Setups/Retrieves priority of current thread
    KMTX_SETER_GETER(KeiraThreadPriority, ktPriority, ktLock);
    // Setups/Retrieves stack size of current thread. Not applyable in thread
    // runtime
    KMTX_SETER_GETER(uint32_t, ktStackSize, ktLock);
    // Setups/Retrieves CPU core to be pinned to. Not applyable in thread
    // runtime
    KMTX_SETER_GETER(int, ktCore, ktLock);
    //=======================================================================
    //  Thread stats/states/information
    //=======================================================================
    // Retrieves current thread state
    const KeiraThreadState getState();
    // Retrieves current thread name
    const char* getName();
    // Setups current thread name. Not applyable in thread runtime
    void setName(const char* ktName);
    // Setups/Retrives Keira thread type
    KMTX_SETER_GETER(KeiraThreadType, ktType, ktLock);
    // Get FreeRTOS task handle
    KMTX_GETER(TaskHandle_t, ktTaskHandle, ktLock);
    //-----------------------------------------------------------------------
    // TODO: mem/etc...
    /////////////////////////////////////////////////////////////////////////

    //=======================================================================
    //  List of callback setters ( Callback API )
    //=======================================================================
    // Thread about to be started             [ Launched in a thread manager ]
    void setupOnStartCallback(KeiraCallback clbk, KeiraCallbackData data) KT_REG_CLBK(KT_ON_START_CLBK);
    // Thread about to be suspended           [ Launched in a thread manager ]
    void setupOnSuspendCallback(KeiraCallback clbk, KeiraCallbackData data) KT_REG_CLBK(KT_ON_SUSPEND_CLBK);
    // Thread about to be resumed             [ Launched in a thread manager ]
    void setupOnResumeCallback(KeiraCallback clbk, KeiraCallbackData data) KT_REG_CLBK(KT_ON_RESUME_CLBK);
    // Thread about to be terminated          [ Launched in a thread manager ]
    void setupOnStopCallback(KeiraCallback clbk, KeiraCallbackData data) KT_REG_CLBK(KT_ON_STOP_CLBK);
    // Thread ends normaly                    [ Launched in a thread ]
    void setupOnExitCallback(KeiraCallback clbk, KeiraCallbackData data) KT_REG_CLBK(KT_ON_EXIT_CLBK);
    // Thread entry point                     [ Launched in a thread ]
    void setupOnEntryCallback(KeiraCallback clbk, KeiraCallbackData data) KT_REG_CLBK(KT_THREAD_CLBK);
    // Clear callback table
    void clearCallbacks();
    /////////////////////////////////////////////////////////////////////////
protected:
    //=======================================================================
    //  List of overridable methods ( Nested API )
    //=======================================================================
    // Thread about to be started             [ Launched in a thread manager ]
    virtual void onStart() KT_IMPL;
    // Thread about to be suspended           [ Launched in a thread manager ]
    virtual void onSuspend() KT_IMPL;
    // Thread about to be resumed             [ Launched in a thread manager ]
    virtual void onResume() KT_IMPL;
    // Thread about to be terminated          [ Launched in a thread manager ]
    virtual void onStop() KT_IMPL;
    // Thread ends normaly                    [ Launched in a thread ]
    virtual void onExit() KT_IMPL;
    // Thread entry point                     [ Launched in a thread ]
    virtual void run() KT_IMPL;
    /////////////////////////////////////////////////////////////////////////

    // Immediate thread exit. Can be run from run() or onEntryCallback
    void exit();

private:
    //=======================================================================
    //  FreeRTOS task management to be done from thread manager
    //=======================================================================
    void start();
    void suspend();
    void resume();
    void stop();
    /////////////////////////////////////////////////////////////////////////
    //=======================================================================
    //  KeiraThread internal wrapper around thread
    //=======================================================================
    void _run();
    /////////////////////////////////////////////////////////////////////////

    //=======================================================================
    //  KeiraThread options and data
    //=======================================================================
    std::vector<KeiraThreadCallbackInternal> ktClbkTable;
    KeiraThreadType ktType = KT_THREAD;
    SemaphoreHandle_t ktLock = xSemaphoreCreateMutex();
    /////////////////////////////////////////////////////////////////////////

    //=======================================================================
    //  FreeRTOS options and data:
    //=======================================================================
    TaskHandle_t ktTaskHandle = NULL;
    char ktName[KT_NAME_MAX] = {};
    uint32_t ktStackSize = KT__STACK;
    int ktCore = KT__CORE;
    KeiraThreadPriority ktPriority = KT_PRIO_DEFAULT;
    KeiraThreadState ktState = KTS_INVALID;
    /////////////////////////////////////////////////////////////////////////
    //=======================================================================
    // TODO: STDIO File streams. Might be useful for pipes? :D
    //=======================================================================
    //    FILE *stdin;
    //    FILE *stdout;
    //    FILE *stderr;
};
