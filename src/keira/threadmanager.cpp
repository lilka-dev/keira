#include "threadmanager.h"

ThreadManager::~ThreadManager() {
    vSemaphoreDelete(lock);
}

ThreadManager::ThreadManager() {
    setName(KEIRA_THREADMANAGER_NAME);
    setktPriority(KEIRA_THREADMANAGER_PRIORITY);
    setktCore(KEIRA_THREADMANAGER_CORE);
}

void ThreadManager::spawn(KeiraThread* thread, bool autoSuspend) {
    KMTX_LOCK(lock);
    // Add new thread to launch
    threadsToRun.push_back(thread);

    KMTX_UNLOCK(lock);

    K_TMG_DBG lilka::serial.log("Added thread %s in threadsToRun", thread->getName());

    // Auto suspend if needed
    if (autoSuspend) {
        TaskHandle_t currentTask = xTaskGetCurrentTaskHandle();
        vTaskSuspend(currentTask);
    }
};

// TODO: improve via utilzing compile time crc32
KeiraThread* ThreadManager::operator[](const char* name) {
    KMTX_LOCK(lock);
    for (auto& thread : threads) {
        if (strcmp(thread->getName(), name) == 0) {
            KMTX_UNLOCK(lock);
            return thread;
        }
    }

    KMTX_UNLOCK(lock);

    return NULL;
}

void ThreadManager::threadsClean() {
    KMTX_LOCK(lock);

    // Terminate exiting threads
    if (!threads.empty())
        for (auto thread = threads.begin(); thread < threads.end();) {
            if ((*thread)->getState() == KTS_EXITING) {
                auto curThread = *thread;
                thread = threads.erase(thread);
                delete curThread;
            } else thread++;
        }

    KMTX_UNLOCK(lock);
}

void ThreadManager::threadsRun() {
    KMTX_LOCK(lock);

    // Launch new threads
    for (auto& thread : threadsToRun) {
        thread->start();
        threads.push_back(thread);
    }
    threadsToRun.clear();

    KMTX_UNLOCK(lock);
}

void ThreadManager::run() {
    while (true) {
        // Terminate exiting
        threadsClean();
        // Launch new
        threadsRun();
        vTaskDelay(pdMS_TO_TICKS(KEIRA_THREADMANAGER_UPDATE_DELAY));
    }
}
