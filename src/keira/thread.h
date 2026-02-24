#pragma once
/////////////////////////////////////////////////////////////////////////////
// Unified class for all FreeRTOS Tasks
/////////////////////////////////////////////////////////////////////////////
//
// Purpose of this component to unify all FreeRTOS task management in
// KeiraOS
//
// Component supports overriding via Nested API or as a building block
// itself via Callback API or both
//
// Callback API supports specifying as well as infinity callbacks for each
// thread state. Callbacks would be launched in their registration order
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// KeiraThread lifecycle:
/////////////////////////////////////////////////////////////////////////////
// Start -> ( [ Suspend/Resume/Working ] * n ) -> [Stop/Exit]
/////////////////////////////////////////////////////////////////////////////

// Uncomment this line to get debugging information
//#define KEIRA_THREAD_DEBUG
#include "keira/bits/thread.h"

#include <vector>

// Simplify callback and their data casting
#define KT_CLBK_CAST(X)      reinterpret_cast<KeiraCallback>(X);
#define KT_CLBK_DATA_CAST(X) reinterpret_cast<KeiraCallbackData>(X);

// To be set in derived classes (Nested API)
// Specifies which system component would control current thread
// management (a.k.a. system task)
typedef enum {
    KT_THREAD,
    KT_APP,
    KT_SERVICE,
    // KT_CMD_APP Maybe sometime in future
} KeiraThreadType;

// Thread states
typedef enum { KTS_RUNING, KTS_BLOCKED, KTS_EXITING } KeiraThreadState;

class KeiraThread {
    // Constructors
    KeiraThread(KeiraCallback clbk, KeiraCallbackData data = NULL);

    //=======================================================================
    //  FreeRTOS task management to be done from system task
    //=======================================================================
    void start();
    void suspend();
    void resume();
    void stop();
    //=======================================================================
public:
    //=======================================================================
    //  Thread settings
    //=======================================================================
    // Setups priority of current thread
    void setPriority();
    // Setups stack size of current thread. Not applyable in thread runtime
    void setStackSize();
    // Setups CPU core to be pinned to. Not applyable in thread runtime
    void setCore();
    /////////////////////////////////////////////////////////////////////////
    //=======================================================================
    // Get current thread state
    const KeiraThreadsState getState();
    /////////////////////////////////////////////////////////////////////////
    //=======================================================================
    //  List of callback setters ( Callback API )
    //=======================================================================
    // Thread about to be started             [ Launched in a system task ]
    void setupOnStartCallback(KeiraCallback clbk, KeiraCallbackData data) KT_REG_CLBK(KT_ON_START_CLBK);
    // Thread about to be suspended           [ Launched in a system task ]
    void setupOnSuspendCallback(KeiraCallback clbk, KeiraCallbackData data) KT_REG_CLBK(KT_ON_SUSPEND_CLBK);
    // Thread about to be resumed             [ Launched in a system task ]
    void setupOnResumeCallback(KeiraCallback clbk, KeiraCallbackData data) KT_REG_CLBK(KT_ON_RESUME_CLBK);
    // Thread about to be terminated          [ Launched in a system task ]
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
    // Thread about to be started             [ Launched in a system task ]
    virtual void onStart() KT_IMPL;
    // Thread about to be suspended           [ Launched in a system task ]
    virtual void onSuspend() KT_IMPL;
    // Thread about to be resumed             [ Launched in a system task ]
    virtual void onResume() KT_IMPL;
    // Thread about to be terminated          [ Launched in a system task ]
    virtual void onStop() KT_IMPL;
    // Thread ends normaly                    [ Launched in a thread ]
    virtual void onExit() KT_IMPL;
    // Thread entry point                     [ Launched in a thread ]
    virtual void run() KT_IMPL;
    /////////////////////////////////////////////////////////////////////////

private:
    // KeiraThread internal wrapper
    void _run();
    void exit();

    // Place to store all callbacks. User can use as many callbacks as needed
    std::vector<KeiraThreadCallbackInternal> clbkTable;

    KeiraThreadType threadType = KT_THREAD;

    // TODO: STDIO File streams
    //    FILE *stdin;
    //    FILE *stdout;
    //    FILE *stderr;
};

#undef KT_REG_CLBK // from src/keira/bits/thread.h
