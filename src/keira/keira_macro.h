#pragma once
///////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  Keira OS Header file
//
///////////////////////////////////////////////////////////////////////////////////////////////////////

//
// A Place to store all wonderful macro magic for further reuse
// and tips of it's usage
//

//=============================================   DEBUG   =============================================

// So far Keira OS supports various features to provide a bit easier debugging for custom apps.
// Most of those come in a form of togglable macro, which you can use together with
// PLATFORMIO_BUILD_FLAGS env variable, to set those options before Keira OS buildage

// example ============================================================================================
// PLATFORMIO_BUILD_FLAGS='-DFMANAGER_DEBUG' pio run -t upload
// would define macro FMANAGER_DEBUG and build a firmware with it
//=====================================================================================================

// Same thing could be also set directly in platformio.ini recipe file using build_flags option

// Watchdog service

// KEIRA_WATCHDOG allow u to run Watchdog serice to output reports about stack/memory usage of all
// services/apps
// WATCHDOG_UPDATE_TIME controls amount of time between reports(in milliseconds)

// Custom app autorun
// KEIRA_DEBUG_APP and KEIRA_DEBUG_APP_PARAMS allow you to control app to be run first, before
// default launcher

// example ============================================================================================
// PLATFORMIO_BUILD_FLAGS='-DKEIRA_DEBUG_APP=FileManagerApp -DKEIRA_DEBUG_APP_PARAMS=\"/sd\"'
// would allow u to autorun FileManagerApp and pass to it path to /sd folder
//=====================================================================================================

// Exit/Entry point loggage, add it to begin/end of method/function to get a comprehensive log
// about calls
#define LEP  lilka::serial.log("==> %s:%d %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__);
#define LEXP lilka::serial.log("<== %s:%d %s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__);

//
// Note: each app/context have to provide own way to make it togglable
// and never to be used in production build
//

// example ============================================================================================

// #ifdef TXT_VIEWER_DEBUG
// #    define TXT_DBG if (1)
// #else
// #    define TXT_DBG if (0)
// #endif

// Usage:
//
// void TxtView::setFont(const uint8_t* font) {
//     TXT_DBG LEP;
//     this->font = font;
// }

// Usage it everywhere isn't guaranteed, simply cause some functions may be run too often
//=====================================================================================================

//===========================================   END DEBUG   ===========================================