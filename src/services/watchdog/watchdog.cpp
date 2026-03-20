#include "watchdog.h"
#include "keira/keira.h"

#ifdef KEIRA_WATCHDOG
REG_SERVICE("watchdog", WatchdogService, true);
#endif

char TASK_STATE_TO_STR[][8] = {"Running", "Ready", "Blocked", "Suspend", "Deleted", "Invalid"};

WatchdogService::~WatchdogService() {
    vSemaphoreDelete(xStats);
}

WatchdogService::WatchdogService() {
    setktStackSize(8192);
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
    KMTX_LOCK(xStats);
    WatchdogTaskData wtData = {};
    wtData.type = type;
    wtData.handle = handle;
    wtData.minFreeStack = uxTaskGetStackHighWaterMark(handle);
    wtData.name = pcTaskGetName(handle);
    wtData.priority = uxTaskPriorityGet(handle);
    wtData.affinity = xTaskGetAffinity(handle);
    lilka::serial.log("Adding task %s to watchdog", wtData.name.c_str());
    taskStats.push_back(wtData);
    KMTX_UNLOCK(xStats);
}

void WatchdogService::addCurrentTask(TaskType type) {
    addTask(xTaskGetCurrentTaskHandle(), type);
}

void WatchdogService::run() {
    while (1) {
        // Get free stack memory

        lilka::serial.log("====================== WATCHDOG DATA ==========================");
        lilka::serial.log("CurFree\tMinFree\tType\tPrio\tCore\tState\tName");
        lilka::serial.log("===============================================================");
        KMTX_LOCK(xStats); // lock unlock

        for (auto ts = taskStats.begin(); ts < taskStats.end();) {
            auto taskState = ts->handle ? eTaskGetState(ts->handle) : eDeleted;

            auto currentFreeStack = 0;

            // Update stats
            if (!(taskState == eInvalid || taskState == eDeleted)) {
                currentFreeStack = uxTaskGetStackHighWaterMark(ts->handle);
                ts->minFreeStack = ts->minFreeStack > currentFreeStack ? currentFreeStack : ts->minFreeStack;
                ts->minFreeStack = uxTaskGetStackHighWaterMark(ts->handle);
                ts->priority = uxTaskPriorityGet(ts->handle);
                ts->affinity = xTaskGetAffinity(ts->handle);
            }
            // Display stats
            lilka::serial.log(
                "%u\t%u\t%s\t%u\t%s\t%s\t%s",
                currentFreeStack,
                ts->minFreeStack,
                taskTypeToString(ts->type).c_str(),
                ts->priority,
                affinityToString(ts->affinity).c_str(),
                TASK_STATE_TO_STR[taskState],
                ts->name
            );
            // Cleanup not existing tasks
            if ((!ts->handle) || taskState == eInvalid || taskState == eDeleted) {
                ts = taskStats.erase(ts);
            } else ts++;
        }
        KMTX_UNLOCK(xStats);

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
