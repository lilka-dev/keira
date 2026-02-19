#include <csetjmp>

#include <lua.hpp>
#include "clib/u8g2.h"
#include <lilka.h>
#include "keira/keira.h"
#include "luarunner.h"
#include "lualilka_display.h"
#include "lualilka_console.h"
#include "lualilka_controller.h"
#include "lualilka_resources.h"
#include "lualilka_math.h"
#include "lualilka_geometry.h"
#include "lualilka_gpio.h"
#include "lualilka_util.h"
#include "lualilka_buzzer.h"
#include "lualilka_state.h"
#include "lualilka_sdcard.h"
#include "lualilka_wifi.h"
#include "lualilka_imageTransform.h"
#include "lualilka_serial.h"
#include "lualilka_http.h"
#include "lualilka_ui.h"
#include "lualilka_crypto.h"
#include "lualilka_audio.h"
#include "keira/ksound/sound.h"
#define SERIAL_DELAY 1000

jmp_buf stopjmp;

bool pushLilka(lua_State* L) {
    // Pushes the global "lilka" table to the top of the stack.
    // Returns false if the table doesn't exist.
    lua_getglobal(L, "lilka");
    if (!lua_istable(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    return true;
}

bool callInit(lua_State* L) {
    // Calls the "init" function of the "lilka" table.
    // Returns false if the function doesn't exist. Keeps "lilka" on the stack.
    lua_getfield(L, -1, "init");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 1);
        return false;
    }
    int retCode = lua_pcall(L, 0, 0, 0);
    if (retCode) {
        // Stack now contains error message on top.
        // We need to pop two deeper values and keep the error message on top.
        lua_remove(L, -2);
        lua_remove(L, -2);
        // Stack now contains only the error message.
        longjmp(stopjmp, retCode);
    }
    return true;
}

bool callUpdate(lua_State* L, uint32_t delta) {
    // Calls the "update" function of the "lilka" table with the given delta.
    // Returns false if the function doesn't exist and pops "lilka" from the stack.
    lua_getfield(L, -1, "update");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        return false;
    }
    lua_pushnumber(L, ((float)delta) / 1000.0);
    int retCode = lua_pcall(L, 1, 0, 0);
    lua_Debug dbg;
    if (retCode) {
        // Stack now contains error message on top.
        // We need to pop two deeper values and keep the error message on top.
        lua_remove(L, -2);
        lua_remove(L, -2);
        // Stack now contains only the error message.
        longjmp(stopjmp, retCode);
    }
    return true;
}

bool callDraw(lua_State* L) {
    // Calls the "draw" function of the "lilka" table.
    // Returns false if the function doesn't exist and pops "lilka" from the stack.
    lua_getfield(L, -1, "draw");
    if (!lua_isfunction(L, -1)) {
        lua_pop(L, 2);
        return false;
    }
    int retCode = lua_pcall(L, 0, 0, 0);
    if (retCode) {
        // Stack now contains error message on top.
        // We need to pop two deeper values and keep the error message on top.
        lua_remove(L, -2);
        lua_remove(L, -2);
        // Stack now contains only the error message.
        longjmp(stopjmp, retCode);
    }
    return true;
}

AbstractLuaRunnerApp::AbstractLuaRunnerApp(const char* appName) :
    App(appName, 0, 0, lilka::display.width(), lilka::display.height()), L(NULL) {
    setFlags(AppFlags::APP_FLAG_FULLSCREEN);
}

void* lua_smart_alloc(void* ud, void* ptr, size_t osize, size_t nsize) {
    // If there will be less than 32 KB of free RAM after reallocating, use PSRAM allocator.
    (void)ud;
    (void)osize;
    int32_t free_mem = heap_caps_get_free_size(MALLOC_CAP_8BIT);
    if (nsize) {
        uint32_t caps = MALLOC_CAP_8BIT;
        if (free_mem - nsize < 32 * 1024) {
            // Less than 32 KB of free RAM after reallocating. Use PSRAM allocator.
            caps |= MALLOC_CAP_SPIRAM;
        } else {
            // More than 32 KB of free RAM after reallocating. Use regular allocator.
        }
        return heap_caps_realloc(ptr, nsize, caps);
    } else {
        free(ptr);
        return NULL;
    }
}

void AbstractLuaRunnerApp::luaSetup(const char* dir) {
    lilka::serial.log("lua: script dir: %s", dir);

    L = lua_newstate(lua_smart_alloc, NULL);

    lilka::serial.log("lua: init libs");
    luaL_openlibs(L);

    // Store dir in registry with "dir" key
    lua_pushstring(L, dir);
    lua_setfield(L, LUA_REGISTRYINDEX, "dir");

    lilka::serial.log("lua: set path");
    // Set package.path to point to the same directory as the script
    lua_getglobal(L, "package");
    lua_pushstring(L, (String(dir) + "/?.lua").c_str());
    lua_setfield(L, -2, "path");
    lua_pop(L, 1);

    // Set path to C modules to the same directory as the script (I wonder if anyone will ever use this)
    lua_getglobal(L, "package");
    lua_pushstring(L, (String(dir) + "/?.so").c_str());
    lua_setfield(L, -2, "cpath");
    lua_pop(L, 1);

    // Store app in registry with "app" key
    lua_pushlightuserdata(L, this);
    lua_setfield(L, LUA_REGISTRYINDEX, "app");

    lilka::serial.log("lua: register globals");
    lualilka_display_register(L);
    lualilka_console_register(L);
    lualilka_controller_register(L);
    lualilka_resources_register(L);
    lualilka_math_register(L);
    lualilka_geometry_register(L);
    lualilka_gpio_register(L);
    lualilka_util_register(L);
    lualilka_buzzer_register(L);
    lualilka_sdcard_register(L);
    lualilka_wifi_register(L);
    lualilka_imageTransform_register(L);
    lualilka_serial_register(L);
    lualilka_http_register(L);
    lualilka_UI_register_keyboard(L);
    lualilka_UI_register_alert(L);
    lualilka_UI_register_progress(L);
    lualilka_crypto_register(L);
    lualilka_audio_register(L);
    lualilka_state_register(L);

    // lilka::serial.log("lua: init canvas");
    // lilka::Canvas* canvas = new lilka::Canvas();
    // lilka::display.setFont(FONT_10x20);
    // canvas->setFont(FONT_10x20);
    // canvas->begin();
    // Initialize table for image pointers
    lilka::serial.log("lua: init memory for images");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "images");

    lilka::serial.log("lua: init memory for sounds");
    lua_newtable(L);
    lua_setfield(L, LUA_REGISTRYINDEX, "sounds");

    // Set global "lilka" table for user stuff
    lua_newtable(L);
    // Add show_fps attribute to lilka table that defaults to false
    lua_pushboolean(L, false);
    lua_setfield(L, -2, "show_fps");
    lua_setglobal(L, "lilka");
}

void AbstractLuaRunnerApp::luaTeardown() {
    lilka::serial.log("lua: cleanup");

    // Free images from registry
    lua_getfield(L, LUA_REGISTRYINDEX, "images");
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        lilka::Image* image = (lilka::Image*)lua_touserdata(L, -1);
        delete image;
        lua_pop(L, 1);
    }

    // Stop audio playback and free sounds from registry
    lualilka_audio_cleanup();
    lua_getfield(L, LUA_REGISTRYINDEX, "sounds");
    lua_pushnil(L);
    while (lua_next(L, -2) != 0) {
        lilka::Sound* sound = static_cast<lilka::Sound*>(lua_touserdata(L, -1));
        delete sound;
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    // Free canvas from registry
    // lilka::Canvas* canvas = (lilka::Canvas*)lua_touserdata(L, lua_getfield(L, LUA_REGISTRYINDEX, "canvas"));
    // delete canvas;

    lua_close(L);
}

int AbstractLuaRunnerApp::execute() {
    // Calls Lua code that's on top of the stack (previously loaded with luaL_loadfile or luaL_loadstring)

    int jmpCode = setjmp(stopjmp);

    if (jmpCode == 0) {
        // Run script
        canvas->fillScreen(0);
        int retCode = lua_pcall(L, 0, LUA_MULTRET, 0);
        if (retCode) {
            longjmp(stopjmp, retCode);
        }
        queueDraw();

        // Get canvas from registry
        // lua_getfield(L, LUA_REGISTRYINDEX, "canvas");
        // lilka::Canvas* canvas = (lilka::Canvas*)lua_touserdata(L, -1);
        // lua_pop(L, 1);

        if (!pushLilka(L)) {
            // No lilka table - we're done
            lua_pop(L, 1);
            longjmp(stopjmp, 32);
        }

        // Check if lilka.init function exists and call it
        canvas->fillScreen(0);
        if (callInit(L)) {
            queueDraw();
        }

        // Check if lilka.update function exists and call it
        const uint32_t perfectDelta = 1000 / 30;
        uint32_t delta = perfectDelta; // Delta for first frame is always 1/30
        while (true) {
            uint32_t now = millis();

            if (!callUpdate(L, delta) || !callDraw(L)) {
                // No update or draw function - we're done
                lilka::serial.log("lua: no update or draw function");
                longjmp(stopjmp, 32);
            }

            // Check if show_fps is true and render FPS
            lua_getfield(L, -1, "show_fps");
            if (lua_toboolean(L, -1)) {
                canvas->setCursor(24, 24);
                canvas->setTextColor(0xFFFF, 0);
                canvas->print(String("FPS: ") + (1000 / (delta > 0 ? delta : 1)) + "  ");
            }
            lua_pop(L, 1);
            queueDraw();

            lua_gc(L, LUA_GCCOLLECT, 0); // TODO: Use LUA_GCSTEP?

            // display.drawCanvas(canvas);

            // Calculate time spent in update & gargage collection
            // TODO: Split time spent in update, time spent in draw, time spent in GC?
            uint32_t elapsed = millis() - now;
            // If we're too fast, delay to keep 30 FPS
            if (elapsed < perfectDelta) {
                vTaskDelay((perfectDelta - elapsed) / portTICK_PERIOD_MS);
                // delay(perfectDelta - elapsed);
                delta = perfectDelta;
            } else {
                // If we're too slow, don't delay and set delta to elapsed time
                delta = elapsed;
            }
        }
    }

    int retCode = jmpCode;
    if (retCode == 32) {
        // Normal exit through longjmp
        return 0;
    }
    return retCode;
}

LuaFileRunnerApp::LuaFileRunnerApp(String path) : AbstractLuaRunnerApp("Lua file run"), path(path) {
    setStackSize(8192);
}

void LuaFileRunnerApp::run() {
#ifndef LILKA_NO_LUA
    // Get dir name from path (without the trailing slash)
    String dir = path.substring(0, path.lastIndexOf('/'));

    luaSetup(dir.c_str());

    // Load state from file (file name is "path" with .lua replaced with .state)
    String statePath = path.substring(0, path.lastIndexOf('.')) + ".state";
    // Store state path in registry for state.save() and state.reset()
    lua_pushstring(L, statePath.c_str());
    lua_setfield(L, LUA_REGISTRYINDEX, "state_path");
    // Check if state file exists
    if (access(statePath.c_str(), F_OK) != -1) {
        lilka::serial.log("lua: found state file %s", statePath.c_str());
        // Load state from file
        lualilka_state_load(L, statePath.c_str());
    }

    lilka::serial.log("lua: run file");

    int retCode = luaL_loadfile(L, path.c_str()) || execute();

    if (retCode) {
        const char* err = lua_tostring(L, -1);
        alert("Lua", String(K_S_LUA_ERROR) + err);
    }

    // Check if state table exists and save it to file if so
    lua_getglobal(L, "state");
    bool hasState = lua_istable(L, -1);
    lua_pop(L, 1);
    if (hasState) {
        lilka::serial.log("lua: saving state to file %s", statePath.c_str());
        lualilka_state_save(L, statePath.c_str());
    } else {
        lilka::serial.log("lua: no state to save");
    }

    luaTeardown();

    // return retCode;
#endif
}

LuaLiveRunnerApp::LuaLiveRunnerApp() : AbstractLuaRunnerApp("Lua source run") {
    setStackSize(8192);
}

void LuaLiveRunnerApp::run() {
#ifndef LILKA_NO_LUA
    // Drain the serial buffer
    Serial.setTimeout(10);
    while (Serial.available()) {
        Serial.read();
    }

    while (1) {
        if (lilka::controller.getState().a.justPressed) {
            return;
        }

        canvas->setFont(FONT_10x20);
        canvas->setCursor(8, 48);
        canvas->fillScreen(lilka::colors::Black);
        canvas->setTextBound(8, 0, canvas->width() - 16, canvas->height());
        canvas->print(K_S_LUA_AWAIT_CODE_FROM_UART);
        queueDraw();

        // Read serial data
        // Default arduino timeout is set to 1000 ms
        Serial.setTimeout(SERIAL_DELAY / 2);
        String code;
        Serial.setRxBufferSize(LUA_SERIAL_TEMPORARY_BUFFER_RX_SIZE); // 16 KB would be enough for everything
        // Note: shit reads not more than amount of this buffer size, so we temporary increase it to make it larger
        // to then decrease to default. idk, it just doesn't work other way
        // await till first chunk
        while (!Serial.available()) {
            if (lilka::controller.getState().a.justPressed) {
                // restore old rx buffer size
                Serial.setRxBufferSize(256);
                return;
            }
            vTaskDelay(pdMS_TO_TICKS(15));
        }

        auto lastRead = millis();
        while (1) {
            // Await till chunk ready
            // Chunks? haha, forget, it doesn't work, just read one big line
            // while (!Serial.available()) {
            //     // await for SERIAL_DELAY for next chunk
            //     LUA_DBG lilka::serial.log("Awaiting next chunk...");
            //     if (millis() - lastRead > SERIAL_DELAY) {
            //         LUA_DBG lilka::serial.log("Chunk prepared, trying to read");
            //         break;
            //     }
            //     vTaskDelay(pdMS_TO_TICKS(15));  // sleep a while
            // }

            // read chunk
            String line = Serial.readString();
            Serial.flush();
            lastRead = millis();

            if (line.length() == 0) break;
            code += line;
        }

        // Those darn line ends...
        // If code contains \r and \n - replace them with \n
        // If code contains only \r - replace it with \n
        // If code contains only \n - leave it as is
        // why the heck we care about line endings at all?
        if (code.indexOf('\r') != -1) {
            if (code.indexOf('\n') != -1) {
                LUA_DBG lilka::serial.log("Line ends: CR and LF");
                code.replace("\r", "");
            } else {
                LUA_DBG lilka::serial.log("Line ends: CR only");
                code.replace("\r", "\n");
            }
        } else {
            LUA_DBG lilka::serial.log("Line ends: LF only");
        }

        // TODO: This is a temporary fix: https://github.com/espressif/arduino-esp32/issues/9221
        // Probably doesn't matter anymore
        lilka::fileutils.isSDAvailable();

        // Display ui stuff
        canvas->fillScreen(lilka::colors::Green);
        canvas->fillScreen(canvas->color565(128, 128, 0));
        canvas->setCursor(8, 180);
        canvas->print(K_S_LUA_DOWNLOAD);
        queueDraw();

        LUA_DBG lilka::serial.log("Read %d bytes of code", code.length());
        LUA_DBG lilka::serial.log(code.c_str());
        // restore old rx buffer size, and allowing to use our memory again. hehe
        Serial.setRxBufferSize(256);

        // Run the code
        execSource(code);
    }

#endif
}

void LuaLiveRunnerApp::execSource(String source) {
#ifndef LILKA_NO_LUA
    luaSetup(lilka::fileutils.getSDRoot().c_str());

    lilka::serial.log("lua: run source");

    int retCode = luaL_loadstring(L, source.c_str()) || execute();

    if (retCode) {
        const char* err = lua_tostring(L, -1);
        alert("Lua", String(K_S_LUA_ERROR) + err);
    }

    luaTeardown();
#endif
}

LuaReplApp::LuaReplApp() : AbstractLuaRunnerApp("Lua REPL") {
    setStackSize(8192);
}

void LuaReplApp::run() {
#ifndef LILKA_NO_LUA
    luaSetup(lilka::fileutils.getSDRoot().c_str()); // TODO: hard-coded

    canvas->setFont(FONT_10x20);
    canvas->setCursor(8, 48);
    canvas->fillScreen(lilka::colors::Black);
    canvas->setTextBound(8, 0, canvas->width() - 16, canvas->height());
    canvas->print("Lua REPL\n\n");
    canvas->print(K_S_LUA_REPL_AWAIT_CODE_FROM_UART);
    queueDraw();

    lilka::serial.log("lua: start REPL");

    // TODO: This is a temporary fix: https://github.com/espressif/arduino-esp32/issues/9221
    lilka::fileutils.initSD();

    bool quit = false;
    while (!quit) {
        String input;
        bool eol = false;
        while (!eol) {
            if (lilka::controller.getState().a.justPressed) {
                quit = true;
                break;
            }
            if (Serial.available()) {
                char c = Serial.read();
                // If backspace
                if (c == 8) {
                    if (input.length() > 0) {
                        input.remove(input.length() - 1);
                        Serial.write(c);
                        Serial.write(' ');
                        Serial.write(c);
                    }
                } else {
                    input += c;
                    Serial.write(c);
                    if (c == 13) {
                        Serial.write(10);
                    }
                }
            }
            if (input.endsWith("\n") || input.endsWith("\r")) {
                eol = true;
            }
        }

        int retCode = luaL_loadstring(L, input.c_str()) || execute();
        lua_gc(L, LUA_GCCOLLECT, 0); // TODO: Use LUA_GCSTEP?

        if (retCode) {
            const char* err = lua_tostring(L, -1);
            lilka::serial.log("lua: error: %s", err);
        }
    }

    lilka::serial.log("lua: stop REPL");
    luaTeardown();
#endif
}
