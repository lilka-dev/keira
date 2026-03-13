#pragma once
// Libraries:
#include <lilka.h> // need to fix it in apps
#include <lilka/display.h>

// TODO: to be moved to statusbar
#define KEIRA_STATUSBAR_HEIGHT 24
#include "keira/thread.h"
#include "keira/bits/app.h"

// Uncomment this line to get debug information
// #define KEIRA_APP_DEBUG

#define APP_CAST(X)  reinterpret_cast<App>(X)
#define APP_PCAST(X) reinterpret_cast<App*>(X)

//============================================================================
// App Flags
//============================================================================
typedef enum : AppFlags_t {
    // App not fully initialized yet
    // This flag not to be used by any app
    APP_FLAG_NONE = 0,

    // Default canvas dimensions
    APP_FLAG_DEFAULT = 1 << 0,
    // Full screen canvas dimensions
    APP_FLAG_FULLSCREEN = 1 << 1,
    // StatusBar canvas dimensions
    APP_FLAG_STATUSBAR = 1 << 2,

    // Flags controlling app drawing
    APP_FLAG_INTERLACED = 1 << 3,
} AppFlags;
//////////////////////////////////////////////////////////////////////////////

class App : public KeiraThread {
    friend class AppManager;

public:
    //========================================================================
    //  App Constructors/Destructors
    //========================================================================
    App(const char* name);
    virtual ~App();
    //========================================================================
    lilka::Canvas* canvas = NULL;
    // Swap canvas and backCanvas, automaticaly changes redraw status
    void queueDraw();

protected:
    //========================================================================
    //  App Flags
    //========================================================================
    void setFlags(AppFlags_t flags);
    AppFlags_t getFlags();
    //========================================================================
    //  App UI Dialogs:
    //========================================================================
    void alert(const String& title, const String& description);
    bool confirm(const String& title, const String& description);
    //////////////////////////////////////////////////////////////////////////
    //========================================================================
    //  App Canvas management
    //========================================================================
    //  Retrieves current redraw status. true indicates redraw needed
    bool getRedraw();
    //  Set redraw status
    void setRedraw(bool redraw);
    // Initializes canvas, recreates canvas on flag changes
    void initCanvas();
    // Clear existing canvases
    void clearCanvas();
    //////////////////////////////////////////////////////////////////////////
    lilka::Canvas* backCanvas = NULL;
    uint32_t frame = 0;
    uint32_t skippedFrames = 0; // Counted only in debug mode
    // TODO: lastFrame, FPS meter
private:
    SemaphoreHandle_t canvasMutex = xSemaphoreCreateMutex();
    bool redraw = false;
    AppFlags_t flags = AppFlags::APP_FLAG_NONE;
};
