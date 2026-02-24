#pragma once
// Internally used by KeiraThread data types, and other sort of stuff which is
// almost never touched for real and have almost zero sense

#include <lilka/serial.h>

#ifdef KEIRA_THREAD_DEBUG
#    define KT_DBG if (1)
#else
#    define KT_DBG if (0)
#endif

typedef void (*KeiraCallback)(void* data);
#define KeiraCallbackData void*

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

const char KEIRA_CLBK_TYPE_TO_STR[][10] = {"Thread", "onStart", "onSuspend", "onResume", "onStop", "onExit"};

// Data type for callbacks inside Keira thread clbkTable
struct KeiraThreadCallbackInternal {
    KeiraThreadCallbackTypeInternal type;
    KeiraCallback clbk;
    KeiraCallbackData data;
};

#define KT_IMPL {};
#define KT_REG_CLBK(CLBK_TYPE)                                                                                      \
    {                                                                                                               \
        KT_DBG lilka::serial.log(                                                                                   \
            "[KThread|%p] Registered %s callback %p, data: %p", this, KEIRA_CLBK_TYPE_TO_STR[CLBK_TYPE], clbk, data \
        );                                                                                                          \
        clbkTable.push_back({.type = CLBK_TYPE, .clbk = clbk, .data = data});                                       \
    }
