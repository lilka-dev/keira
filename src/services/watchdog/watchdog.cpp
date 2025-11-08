#include "watchdog.h"

WatchdogService::WatchdogService() : Service("watchdog") {
}

const String WatchdogService::taskTypeToString(TaskType type) {
    switch (type) {
        case WATCHDOG_TASK_APP:
            return "APP";
        case WATCHDOG_TASK_SERVICE:
            return "SERVICE";
        case WATCHDOG_TASK_MISC:
            return "MISC";
        default:
            return "UNKNOWN";
    }
}

const String WatchdogService::affinityToString(BaseType_t affinity) {
    if (affinity == tskNO_AFFINITY) return "ALL";
    else return StringFormat("CPU%d", affinity);
}

void WatchdogService::addTask(const TaskHandle_t& handle, TaskType type) {
    WatchdogTaskData wtData = {};
    wtData.type = type;
    wtData.handle = handle;
    wtData.minFreeStack = uxTaskGetStackHighWaterMark(handle);
    wtData.name = pcTaskGetName(handle);
    wtData.priority = uxTaskPriorityGet(handle);
    wtData.affinity = xTaskGetAffinity(handle);
    lilka::serial.log("Adding task %s to watchdog", wtData.name.c_str());
    xSemaphoreTake(xStats, portMAX_DELAY);
    taskStats.push_back(wtData);
    xSemaphoreGive(xStats);
}
void WatchdogService::addCurrentTask(TaskType type) {
    addTask(xTaskGetCurrentTaskHandle(), type);
}
void WatchdogService::run() {
    while (1) {
        // Get free stack memory

        lilka::serial.log("================= WATCHDOG DATA ======================");
        lilka::serial.log("CurFree\tMinFree\tType\tPrio\tCore\tName");
        lilka::serial.log("======================================================");
        xSemaphoreTake(xStats, portMAX_DELAY); // lock unlock
        for (auto& ts : taskStats) {
            auto currentFreeStack = uxTaskGetStackHighWaterMark(ts.handle);
            ts.minFreeStack = ts.minFreeStack > currentFreeStack ? currentFreeStack : ts.minFreeStack;
            lilka::serial.log(
                "%u\t%u\t%s\t%u\t%s\t%s",
                currentFreeStack,
                ts.minFreeStack,
                taskTypeToString(ts.type).c_str(),
                ts.priority,
                affinityToString(ts.affinity).c_str(),
                ts.name
            );
        }
        xSemaphoreGive(xStats);

        // Get free RAM

        lilka::serial.log(
            "RAM   : %s/%s free",
            lilka::fileutils.getHumanFriendlySize(ESP.getFreeHeap()).c_str(),
            lilka::fileutils.getHumanFriendlySize(ESP.getHeapSize()).c_str()
        );
        lilka::serial.log(
            "SPIRAM: %s/%s free",
            lilka::fileutils.getHumanFriendlySize(ESP.getFreePsram()).c_str(),
            lilka::fileutils.getHumanFriendlySize(ESP.getPsramSize()).c_str()
        );
        lilka::serial.log("======================================================");
        vTaskDelay(WATCHDOG_UPDATE_TIME / portTICK_PERIOD_MS);
    }
}
