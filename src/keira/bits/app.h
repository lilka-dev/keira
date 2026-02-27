#pragma once

//============================================================================
//  DEBUGING
//============================================================================
#ifdef KEIRA_APP_DEBUG
#    define KAPP_DBG if (1)
#else
#    define KAPP_DBG if (0)
#endif
//============================================================================
//  CASTs simplify
//============================================================================
#define AppFlags_t unsigned int

//============================================================================
//  DRAWING FLAGS
//============================================================================
#define APP_DRAWING_FLAGS (APP_FLAG_DEFAULT | APP_FLAG_STATUSBAR | APP_FLAG_FULLSCREEN)
//============================================================================
//  THREAD SETTINGS
//============================================================================
// == STACK SIZE
#ifndef APP_STACK
#    define APP_DEFAULT_STACK 4096
#endif
//----------------------------------------------------------------------------
// == CPU CORE
#ifndef APP_DEFAULT_CORE
#    define APP_DEFAULT_CORE tskNO_AFFINITY
#endif
//----------------------------------------------------------------------------
// == THREAD NAME
#ifndef APP_DEFAULT_NAME
#    define APP_DEFAULT_NAME "App"
#endif
//----------------------------------------------------------------------------
// == PRIORITY
#ifndef APP_DEFAULT_PRIO
#    define APP_DEFAULT_PRIO KT_PRIO_DEFAULT
#endif
//////////////////////////////////////////////////////////////////////////////
