#pragma once
#include <FreeRTOS.h>

// Mutex debug utils

#ifdef KEIRA_MUTEX_DEBUG

#    define KMTX_MAX_FUNC_LEN 260

typedef struct {
    SemaphoreHandle_t* mtx;
    char funcName[KMTX_MAX_FUNC_LEN];
    size_t line;
} KeiraDebugMtx_t;

extern std::vector<KeiraDebugMtx_t> keiraDebugMutexList;
extern SemaphoreHandle_t keiraDebugMutexSemaphore;

class AcquireMutex {
public:
    // Lock mutex, check on double lock
    explicit AcquireMutex(SemaphoreHandle_t& mtx, const char* func_name, size_t line);
    // Released normally
    void release();
    // Object is out of scope, here we detect if it was released correctly
    ~AcquireMutex();

private:
    // indicates mutex isn't unlocked normally
    bool releasedByUser = false;
    // indicates if mutex was already locked
    bool multiLock = false;
    KeiraDebugMtx_t* pMutexRecord = NULL;
};

// TODO: use new together with std::unique_ptr

#    define KMTX_LOCK(X) AcquireMutex kmtxDbgMutex(X, __PRETTY_FUNCTION__, __LINE__)
#    define KMTX_UNLOCK(X)

#else
#    define KMTX_LOCK(X)   xSemaphoreTake(X, portMAX_DELAY)
#    define KMTX_UNLOCK(X) xSemaphoreGive(X)
#endif
