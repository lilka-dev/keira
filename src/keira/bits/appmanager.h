#pragma once

#ifdef KEIRA_APPMANAGER_DEBUG
#    define K_AMG_DBG if (1)
#else
#    define K_AMG_DBG if (0)
#endif

#define KEIRA_TOAST_INITIALIZER {.message = "", .startTime = 0, .endTime = 0, .mtx = xSemaphoreCreateMutex()}
