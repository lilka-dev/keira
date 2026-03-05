#pragma once
#include <FreeRTOS.h>
#include <lilka/serial.h>

// Mutex debug utils

// Uncomment this line to get debug information about mutexes

#define KEIRA_MUTEX_DEBUG

#ifdef KEIRA_MUTEX_DEBUG

#    define KMTX_MAX_FUNC_LEN 260

typedef struct {
    SemaphoreHandle_t* mtx;
    char funcName[KMTX_MAX_FUNC_LEN];
    size_t line;
    size_t id;
} KeiraDebugMtx_t;

size_t keiraDebugMutexLastId = 0;
extern std::vector<KeiraDebugMtx_t> keiraDebugMutexList;
extern SemaphoreHandle_t keiraDebugMutexSemaphore;

class AcquireMutex {
public:
    // Lock mutex, check on double lock
    explicit AcquireMutex(SemaphoreHandle_t& mtx, const char* funcName, size_t line);
    // Released normally
    void release();
    // Object is out of scope, here we detect if it was released correctly
    ~AcquireMutex();

private:
    // indicates mutex isn't unlocked normally
    bool releasedByUser = false;
    // indicates if mutex was already locked
    bool multiLock = false;

    size_t mtxId = 0;
};

// unique variable name
#    define KMTX_CONCAT_INNER(a, b) a##b
#    define KMTX_CONCAT(a, b)       KMTX_CONCAT_INNER(a, b)
#    define KMTX_LOCK(X)            AcquireMutex KMTX_CONCAT(kmtxDbgMutex_, __LINE__)(X, __PRETTY_FUNCTION__, __LINE__)
#    define KMTX_UNLOCK(X)

#else
#    define KMTX_LOCK(X)   xSemaphoreTake(X, portMAX_DELAY)
#    define KMTX_UNLOCK(X) xSemaphoreGive(X)
#endif
