#pragma once
// Internally used by KeiraThread data types, and other sort of stuff which is
// almost never touched for real and have almost zero sense in appearing that
// frequently on any eyes

// Needed for debuging
#include <lilka/serial.h>
//============================================================================
//  DEBUGING
//============================================================================
#ifdef KEIRA_THREAD_DEBUG
#    define KT_DBG if (1)
#else
#    define KT_DBG if (0)
#endif
//----------------------------------------------------------------------------
// == CALLBACK TYPE STRINGS
const char KEIRA_CLBK_TYPE_ACSTR[][10] = {"Thread", "onStart", "onSuspend", "onResume", "onStop", "onExit"};
//----------------------------------------------------------------------------
// == Debug specific implementations of virtual methods
#ifdef KEIRA_THREAD_DEBUG
// TODO: implement debuging
#    define KT_IMPL {};
#else
#    define KT_IMPL {};
#endif
//////////////////////////////////////////////////////////////////////////////

//============================================================================
// DEFAULT KeiraThread SETTINGS
//============================================================================
// == STACK SIZE
#ifndef KT_DEFAULT_STACK
#    define KT_DEFAULT_STACK 2048
#endif
//----------------------------------------------------------------------------
// == CPU CORE
#ifndef KT_DEFAULT_CORE
#    define KT_DEFAULT_CORE tskNO_AFFINITY
#endif
//----------------------------------------------------------------------------
#ifndef KT_DEFAULT_NAME
#    define KT_DEFAULT_NAME "Thread"
#endif
//////////////////////////////////////////////////////////////////////////////

//============================================================================
// CALLBACK DATA TYPES
//============================================================================
typedef void (*KeiraCallback)(void* data);
//----------------------------------------------------------------------------
#define KeiraCallbackData void*
//----------------------------------------------------------------------------
// == Type of keira callback. Specifies how and when specific callback to launch
typedef enum {
    // To be launched as thread
    KT_THREAD_CLBK,
    // To be launched before RTOS thread->start()
    KT_ON_START_CLBK,
    // To be launched on thread->suspend()
    KT_ON_SUSPEND_CLBK,
    // To be launched on thread->resume()
    KT_ON_RESUME_CLBK,
    // To be launched on thread->stop()
    KT_ON_STOP_CLBK,
    // To be launched on thread exit normally
    KT_ON_EXIT_CLBK
} KeiraThreadCallbackTypeInternal;
//----------------------------------------------------------------------------
// == Data type for callbacks inside Keira thread clbkTable
struct KeiraThreadCallbackInternal {
    KeiraThreadCallbackTypeInternal type;
    KeiraCallback clbk;
    KeiraCallbackData data;
};
//----------------------------------------------------------------------------
// == Callback registration macro
#define KT_REG_CLBK(CLBK_TYPE)                                                                                     \
    {                                                                                                              \
        xSemaphoreTake(ktLock, portMAX_DELAY);                                                                     \
        KT_DBG lilka::serial.log(                                                                                  \
            "[KThread|%p] Registered %s callback %p, data: %p", this, KEIRA_CLBK_TYPE_ACSTR[CLBK_TYPE], clbk, data \
        );                                                                                                         \
        ktClbkTable.push_back({.type = CLBK_TYPE, .clbk = clbk, .data = data});                                    \
        KMTX_UNLOCK(ktLock);                                                                                    \
    }
//////////////////////////////////////////////////////////////////////////////
