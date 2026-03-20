#pragma once
#include "keira/debug.h"

#ifdef KEIRA_SERVICEMANAGER_DEBUG
#    define K_SM_DBG if (1)
#else
#    define K_SM_DBG if (0)
#endif

#ifndef KEIRA_SERVICEMANAGER_CORE
#    define KEIRA_SERVICEMANAGER_CORE 0
#endif

