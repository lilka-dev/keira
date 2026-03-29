#pragma once
///////////////////////////////////////////////////////////////////////////////
// People are a bit afraid of magic, so, let's hide it to be not burnt in fire
///////////////////////////////////////////////////////////////////////////////
#ifdef KEIRA_THREADMANAGER_DEBUG
#    define K_TMG_DBG if (1)
#else
#    define K_TMG_DBG if (0)
#endif

// Spawn/Kill threads not faster than each
#ifndef KEIRA_THREADMANAGER_UPDATE_DELAY
#    define KEIRA_THREADMANAGER_UPDATE_DELAY 40
#endif

#ifndef KEIRA_THREADMANAGER_CORE
#    define KEIRA_THREADMANAGER_CORE 0
#endif

#ifndef KEIRA_THREADMANAGER_PRIORITY
#    define KEIRA_THREADMANAGER_PRIORITY KT_PRIO_IDLE
#endif

#ifndef KEIRA_THREADMANAGER_NAME
#    define KEIRA_THREADMANAGER_NAME "ThreadManager"
#endif