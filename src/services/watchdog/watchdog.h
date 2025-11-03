#pragma once

#include <WiFi.h>
#include "keira/service.h"
#ifndef WATCHDOG_UPDATE_TIME
#    define WATCHDOG_UPDATE_TIME 10000
#endif
typedef enum { WATCHDOG_TASK_APP, WATCHDOG_TASK_SERVICE, WATCHDOG_TASK_MISC } TaskType;

typedef struct {
    TaskHandle_t handle;
    String name;
    uint32_t minFreeStack; // minFree
    TaskType type;
    UBaseType_t priority;
    BaseType_t affinity; // CPU Core
} WatchdogTaskData;

class WatchdogService : public Service {
public:
    WatchdogService();
    const String taskTypeToString(TaskType type);
    const String affinityToString(BaseType_t affinity);
    void addTask(const TaskHandle_t& handle, TaskType type);
    void addCurrentTask(TaskType type);

private:
    void run() override;
    SemaphoreHandle_t xStats = xSemaphoreCreateMutex();
    std::vector<WatchdogTaskData> taskStats;
};
