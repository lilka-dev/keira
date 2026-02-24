#pragma once

#include <Arduino.h>
#include <lilka.h>

#define KEIRA_STATUSBAR_HEIGHT 24

// Uncomment this line to get debug information
//#define KEIRA_APP_DEBUG

#ifdef KEIRA_APP_DEBUG
#    define KAPP_DBG if (1)
#else
#    define KAPP_DBG if (0)
#endif

#define AppFlags_t unsigned int

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

#define APP_DRAWING_FLAGS (APP_FLAG_DEFAULT | APP_FLAG_STATUSBAR | APP_FLAG_FULLSCREEN)

/// Клас, що представляє додаток для Кіри.
///
/// Додатки запускаються за допомогою синглтону AppManager.
///
/// Додаток має визначити принаймні метод run(), який буде викликатися в окремій задачі FreeRTOS.
///
/// При завершенні додатку, AppManager зупиняє задачу та видаляє об'єкт додатку.
///
/// Приклад запуску додатку:
///
/// @code
/// #include <appmanager.h>
/// #include <myapp.h>
///
/// // ...
///
/// AppManager::getInstance()->runApp(new MyApp());
/// @endcode
class App {
    friend class AppManager;

public:
    App(const char* name);
    virtual ~App();
    /// Повідомити ОС, що додаток завершив малювання кадру.
    ///
    /// Цей метод слід викликати після малювання кожного кадру.
    ///
    /// Технічно, цей метод міняє місцями передній буфер (canvas) та задній буфер (backCanvas).
    ///
    /// Якщо цей метод викликається в той час, коли ОС вже малює попередній кадр, то він призведе до нетривалого блокування додатку.
    /// Іншими словами, якщо ваш додаток малює кадри швидше, ніж ОС здатна їх відображати, то цей метод буде час від часу заповільнювати ваш додаток.
    /// Це не проблема, але варто про це пам'ятати.
    void queueDraw();
    const char* getName();

    // Return exiting flag to be handled by AppManager
    bool getExiting();

    eTaskState getState();

    /// Вказівник на передній буфер для малювання.
    ///
    /// Додаток повинен використовувати цей буфер для малювання всієї графіки.
    ///
    /// Після малювання, буфер потрібно відобразити на екрані за допомогою методу queueDraw().
    lilka::Canvas* canvas = NULL;

protected:
    // to be run from appManager
    void start();
    // to be run from appManager
    void suspend();
    // to be run from appManager
    void resume();
    // to be run from appManager
    void stop();

    // Immediate exit from app
    void exit();
    void setCore(int appCore);
    /// Встановити прапорці додатку.
    ///
    /// Наприклад, якщо додаток має відображатися на весь екран, то слід викликати setFlags(APP_FLAG_FULLSCREEN).
    /// @param flags
    void setFlags(AppFlags_t flags);
    AppFlags_t getFlags();
    /// Встановити розмір стеку задачі додатку.
    ///
    /// За замовчуванням, розмір стеку задачі дорівнює 4096 байт. Проте деякі додатки можуть вимагати більший розмір стеку.
    /// Врахуйте що значення має бути кратним 2048 для запобігання фрагментації пам'яті
    void setStackSize(uint32_t stackSize);
    /// Допоміжні методи для роботи зі сповіщеннями
    void alert(const String& title, const String& description);
    bool confirm(const String& title, const String& description);

    bool needsRedraw();
    void markClean();
    void forceRedraw();

    void acquireBackCanvas();
    void releaseBackCanvas();

    lilka::Canvas* backCanvas = NULL;
    uint64_t frame;

private:
    void _run();
    /// Основний код додатку.
    ///
    /// Цей метод викликається в окремій задачі FreeRTOS.
    ///
    /// Додаток завершується, коли цей метод завершується або робить return.
    virtual void run() = 0;
    /// Цей метод викликається операційною системою, коли вона збирається зупинити ваш додаток.
    virtual void onSuspend() {
    }
    /// Цей метод викликається операційною системою, коли вона збирається відновити роботу вашого додатку.
    virtual void onResume() {
    }
    /// Цей метод викликається операційною системою, коли вона збирається зупинити ваш додаток.
    virtual void onExit() {
    }
    /// Initializes canvas, recreates canvas on flag changes
    void initCanvas();
    const char* name;
    SemaphoreHandle_t backCanvasMutex;
    bool isDrawQueued;
    int appCore;
    AppFlags_t flags = AppFlags::APP_FLAG_NONE;
    TaskHandle_t taskHandle;
    uint32_t stackSize;

    bool exiting = false;
};
