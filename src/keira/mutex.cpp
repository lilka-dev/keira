#include "keira/mutex.h"
// xSemaphoreCreateMutex
#ifdef KEIRA_MUTEX_DEBUG

AcquireMutex::AcquireMutex(SemaphoreHandle_t& mtx, const char* funcName, size_t line) {
    // Lock keiraDebugMutexList
    xSemaphoreTake(keiraDebugMutexSemaphore, portMAX_DELAY);

    // seek if mutex already in a locked list
    for (auto& kmtx : keiraDebugMutexList) {
        if (kmtx->mtx == &mtx) {
            multiLock = true;
            lilka::serial.log("Double lock %s:%d, first locked at %s:%d", funcName, line, kmtx.funcName, kmtx.line);
            break;
        }
    }

    // Create new mutex record
    KeiraDebugMtx_t newMtxRecord = {.mtx = &mtx, .line = line, .id = keiraDebugMutexLastId++};
    strcpy(newMtxRecord.funcName, funcName);

    // Store mutexRecord
    keiraDebugMutexList.push_back(newMtxRecord);
    mtxId = newMtxRecord.id;

    // lock mutex
    xSemaphoreTake(mtx, portMAX_DELAY);
    // Unlock keiraDebugMutexList
    xSemaphoreGive(keiraDebugMutexList);
}

void AcquireMutex::release() {
    // Lock keiraDebugMutexList
    xSemaphoreTake(keiraDebugMutexSemaphore, portMAX_DELAY);
    releasedByUser = true;

    // Remove mutex from table
    for (auto pkmtx = keiraDebugMutexList.begin(); pkmtx < keiraDebugMutexList.end(); pkmtx++) {
        if (mtxId == pkmtx->id) {
            // Release mutex
            xSemaphoreGive(*(pkmtx.mtx));
            lilka::serial.log("Released ID%d %s:%d", pkmtx.id, pkmtx.line, pkmtx.line);
            keiraDebugMutexList.erase(pkmtx);
        }
    }

    // Unlock keiraDebugMutexList
    xSemaphoreGive(keiraDebugMutexList);
}

AcquireMutex::~AcquireMutex() {
    // Lock keiraDebugMutexList
    xSemaphoreTake(keiraDebugMutexSemaphore, portMAX_DELAY);

    // Validate mutexRelease
    if (!releasedByUser) {
        for (auto pkmtx = keiraDebugMutexList.begin(); pkmtx < keiraDebugMutexList.end(); pkmtx++) {
            if (mtxId == pkmtx->id) {
                lilka::serial.err("Mutex not released %s:%d", pkmtx.funcName, pkmtx.line);
            }
        }
        // Auto release, seek for other mistakes
        this->release();
    }

    // Unlock keiraDebugMutexList
    xSemaphoreGive(keiraDebugMutexList);
}

#endif
