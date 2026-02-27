#include "threadmanager.h"
#

ThreadManager::ThreadManager() {
    //    setName(KEIRA_THREADMANAGER_NAME);
    setPriority(KEIRA_THREADMANAGER_PRIORITY);
    setCore(KEIRA_THREADMANAGER_CPU);
}

void ThreadManager::spawn(KeiraThread* thread, bool autoSuspend) {
    xSemaphoreTake(lock, portMAX_DELAY);
    // Add new thread to launch
    threadsToRun.push_back(thread);

    xSemaphoreGive(lock);

    K_TMG_DBG lilka::serial.log("Added thread %s in threadsToRun", thread->getName());
};

KeiraThread* ThreadManager::operator[](const char* name) {
    xSemaphoreTake(lock, portMAX_DELAY);
    for (auto& thread : threads) {
        if (strcmp(thread->getName(), name) == 0) {
            xSemaphoreGive(lock);
            return thread;
        }
    }

    xSemaphoreGive(lock);

    return NULL;
}

void ThreadManager::threadsClean() {
    xSemaphoreTake(lock, portMAX_DELAY);

    // Terminate exiting threads
    if (!threads.empty())
        for (auto thread = threads.begin(); thread < threads.end();) {
            if ((*thread)->getState() == KTS_EXITING) {
                auto curThread = *thread;
                threads.erase(thread);
                delete curThread;
            } else thread++;
        }

    xSemaphoreGive(lock);
}

void ThreadManager::threadsRun() {
    xSemaphoreTake(lock, portMAX_DELAY);

    // Launch new threads
    for (auto& thread : threadsToRun) {
        thread->start();
        threads.push_back(thread);
    }
    threadsToRun.clear();

    xSemaphoreGive(lock);
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
