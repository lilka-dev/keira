#include "keira/mutex.h"
// xSemaphoreCreateMutex
#ifdef KEIRA_MUTEX_DEBUG

AcquireMutex::AcquireMutex(SemaphoreHandle_t& mtx, const char* func_name, size_t line) {
    // Lock keiraDebugMutexList
    xSemaphoreTake(keiraDebugMutexSemaphore, portMAX_DELAY);

    // seek if mutex already in a locked list
    for (auto& kmtx : keiraDebugMutexList) {
        if (kmtx->mtx == &mtx) {
            multiLock = true;
            lilka::serial.log("Double lock %s:%d, first locked at %s:%d", func_name, line, kmtx.funcName, kmtx.line);
            break;
        }
    }

    // Create new mutex record
    KeiraDebugMtx_t newMtxRecord = {.mtx = &mtx, .line = line};
    strcpy(newMtxRecord.func_name, func_name);

    // Store mutexRecord
    keiraDebugMutexList.push_back(newMtxRecord);
    pMutexRecord = keiraDebugMutexList.back();

    // lock mutex
    xSemaphoreTake(mtx, portMAX_DELAY);
    // Unlock keiraDebugMutexList
    xSemaphoreGive(keiraDebugMutexList);
}

void AcquireMutex::release() {
    // Lock keiraDebugMutexList
    xSemaphoreTake(keiraDebugMutexSemaphore, portMAX_DELAY);
    releasedByUser = true;

    // Release mutex
    xSemaphoreGive(*(this->pMutexRecord->mtx));

    // Remove mutex from table
    for (auto pkmtx = keiraDebugMutexList.begin(); pkmtx < keiraDebugMutexList.end(); pkmtx++) {
        if (pMutexRecord == pkmtx) {
            lilka::serial.log("Released %s:%d, Locked at %s:%d", func_name, line, pkmtx.funcName, pkmtx->line);
            keiraDebugMutexList.erase(pkmtx);
        }
    }

    // Unlock keiraDebugMutexList
    xSemaphoreGive(keiraDebugMutexList);
}

void AcquireMutex::~AcquireMutex() {
    // Lock keiraDebugMutexList
    xSemaphoreTake(keiraDebugMutexSemaphore, portMAX_DELAY);

    // Validate mutexRelease
    if (!releasedByUser) {
        lilka::serial.err("Mutex not released %s:%d", this->pMutexRecord.funcName, this->pMutexRecord.line);
        // Auto release, seek for other mistakes
        this->release();
    }

    // Unlock keiraDebugMutexList
    xSemaphoreGive(keiraDebugMutexList);
}

#endif
