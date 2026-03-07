#pragma once

#ifdef KEIRA_APPMANAGER_DEBUG
#    define K_AMG_DBG if (1)
#else
#    define K_AMG_DBG if (0)
#endif

#define KEIRA_TOAST_INITIALIZER {.message = "", .startTime = 0, .endTime = 0, .mtx = xSemaphoreCreateMutex()}

//============================================================================
//  THREAD SETTINGS
//============================================================================
// == STACK SIZE
#ifndef APPMANAGER_STACK
#    define APPMANAGER_STACK 4096
#endif
//----------------------------------------------------------------------------
// == CPU CORE
#ifndef APPMANAGER_CORE
#    define APPMANAGER_CORE tskNO_AFFINITY
#endif
//----------------------------------------------------------------------------
// == THREAD NAME
#ifndef APPMANAGER_NAME
#    define APPMANAGER_NAME "AppManager"
#endif
//----------------------------------------------------------------------------
// == PRIORITY
#ifndef APPMANAGER_PRIO
#    define APPMANAGER_PRIO KT_PRIO_DEFAULT
#endif
//////////////////////////////////////////////////////////////////////////////
