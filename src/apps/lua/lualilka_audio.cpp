#include <AudioGeneratorMOD.h>
#include <AudioGeneratorWAV.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorAAC.h>
#include <AudioGeneratorFLAC.h>
#include <AudioOutputI2S.h>
#include <AudioFileSourcePROGMEM.h>

#include <lilka.h>

#include "lualilka_audio.h"

// Command types for the audio task
typedef enum {
    AUDIO_CMD_STOP,
    AUDIO_CMD_PAUSE,
    AUDIO_CMD_RESUME,
    AUDIO_CMD_SET_GAIN,
} AudioCommandType;

typedef struct {
    AudioCommandType type;
    float gain;
} AudioCommand;

// Static state — only one Lua runtime at a time
static AudioGenerator* audioGen = NULL;
static AudioOutputI2S* audioOut = NULL;
static AudioFileSourcePROGMEM* audioSource = NULL;
static TaskHandle_t audioTaskHandle = NULL;
static QueueHandle_t audioCommandQueue = NULL;
static SemaphoreHandle_t audioMutex = NULL;
static volatile bool audioIsPlaying = false;
static volatile bool audioIsPaused = false;
static volatile bool audioIsFinished = false;
static float audioGain = -1.0f;

static void stopTaskAndClean() {
    if (audioTaskHandle == NULL) {
        return;
    }
    // Send stop command
    AudioCommand cmd = {.type = AUDIO_CMD_STOP, .gain = 0};
    xQueueSend(audioCommandQueue, &cmd, portMAX_DELAY);
    // Wait for task to finish (up to 500ms)
    for (int i = 0; i < 50; i++) {
        if (audioIsFinished) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    // Fallback: force delete if still running
    if (!audioIsFinished && audioTaskHandle != NULL) {
        vTaskDelete(audioTaskHandle);
    }
    audioTaskHandle = NULL;

    if (audioGen) {
        delete audioGen;
        audioGen = NULL;
    }
    if (audioSource) {
        delete audioSource;
        audioSource = NULL;
    }
    if (audioOut) {
        delete audioOut;
        audioOut = NULL;
    }
}

static void audioTask(void* arg) {
    while (1) {
        // Check for commands
        AudioCommand cmd;
        if (xQueueReceive(audioCommandQueue, &cmd, 0) == pdTRUE) {
            switch (cmd.type) {
                case AUDIO_CMD_STOP:
                    if (audioGen) {
                        audioGen->stop();
                    }
                    xSemaphoreTake(audioMutex, portMAX_DELAY);
                    audioIsPlaying = false;
                    audioIsPaused = false;
                    audioIsFinished = true;
                    xSemaphoreGive(audioMutex);
                    vTaskDelete(NULL);
                    return;
                case AUDIO_CMD_PAUSE:
                    xSemaphoreTake(audioMutex, portMAX_DELAY);
                    audioIsPaused = true;
                    xSemaphoreGive(audioMutex);
                    break;
                case AUDIO_CMD_RESUME:
                    xSemaphoreTake(audioMutex, portMAX_DELAY);
                    audioIsPaused = false;
                    xSemaphoreGive(audioMutex);
                    break;
                case AUDIO_CMD_SET_GAIN:
                    xSemaphoreTake(audioMutex, portMAX_DELAY);
                    audioGain = cmd.gain;
                    if (audioGain < 0) audioGain = 0;
                    if (audioGain > 4) audioGain = 4;
                    xSemaphoreGive(audioMutex);
                    audioOut->SetGain(audioGain);
                    break;
            }
        }

        xSemaphoreTake(audioMutex, portMAX_DELAY);
        bool paused = audioIsPaused;
        bool finished = audioIsFinished;
        xSemaphoreGive(audioMutex);

        if (finished) {
            vTaskDelete(NULL);
            return;
        }

        if (!paused) {
            if (!audioGen->loop()) {
                // Track finished
                audioGen->stop();
                xSemaphoreTake(audioMutex, portMAX_DELAY);
                audioIsPlaying = false;
                audioIsFinished = true;
                xSemaphoreGive(audioMutex);
                vTaskDelete(NULL);
                return;
            }
        }
        taskYIELD();
    }
}

void lualilka_audio_stop_playback() {
    stopTaskAndClean();
}

int lualilka_audio_play(lua_State* L) {
    // Arg: table with "pointer" (lightuserdata) and "size" (number)
    luaL_checktype(L, 1, LUA_TTABLE);
    lua_getfield(L, 1, "pointer");
    if (!lua_islightuserdata(L, -1)) {
        return luaL_error(L, "Невірний аудіо-ресурс");
    }
    uint8_t* data = static_cast<uint8_t*>(lua_touserdata(L, -1));
    lua_pop(L, 1);
    lua_getfield(L, 1, "size");
    int size = luaL_checkinteger(L, -1);
    lua_pop(L, 1);
    lua_getfield(L, 1, "type");
    const char* type = luaL_checkstring(L, -1);
    lua_pop(L, 1);

    // Stop any currently playing audio
    stopTaskAndClean();

    // Create command queue and mutex if not yet created
    if (audioCommandQueue == NULL) {
        audioCommandQueue = xQueueCreate(8, sizeof(AudioCommand));
    } else {
        xQueueReset(audioCommandQueue);
    }
    if (audioMutex == NULL) {
        audioMutex = xSemaphoreCreateMutex();
    }

    // Create audio output with I2S pins
    audioOut = new AudioOutputI2S();
    audioOut->SetPinout(LILKA_I2S_BCLK, LILKA_I2S_LRCK, LILKA_I2S_DOUT);

    // Create source from RAM buffer
    audioSource = new AudioFileSourcePROGMEM(data, size);

    // Create player based on file type
    if (strcmp(type, "mod") == 0) {
        audioGen = new AudioGeneratorMOD();
    } else if (strcmp(type, "wav") == 0) {
        audioGen = new AudioGeneratorWAV();
    } else if (strcmp(type, "mp3") == 0) {
        audioGen = new AudioGeneratorMP3();
    } else if (strcmp(type, "aac") == 0) {
        audioGen = new AudioGeneratorAAC();
    } else if (strcmp(type, "flac") == 0) {
        audioGen = new AudioGeneratorFLAC();
    } else {
        delete audioSource;
        audioSource = NULL;
        delete audioOut;
        audioOut = NULL;
        return luaL_error(L, "Непідтримуваний формат аудіо: %s", type);
    }
    audioGen->begin(audioSource, audioOut);

    // Set initial gain from system volume
    if (audioGain < 0) {
        audioGain = lilka::audio.getVolume() / 100.0f;
    }
    audioOut->SetGain(audioGain);

    // Reset state
    audioIsPlaying = true;
    audioIsPaused = false;
    audioIsFinished = false;

    // Start background task on core 0
    xTaskCreatePinnedToCore(audioTask, "LuaAudio", 8192, NULL, 1, &audioTaskHandle, 0);

    return 0;
}

int lualilka_audio_stop(lua_State* L) {
    lualilka_audio_stop_playback();
    return 0;
}

int lualilka_audio_pause(lua_State* L) {
    if (audioTaskHandle != NULL && audioIsPlaying && !audioIsPaused) {
        AudioCommand cmd = {.type = AUDIO_CMD_PAUSE, .gain = 0};
        xQueueSend(audioCommandQueue, &cmd, portMAX_DELAY);
    }
    return 0;
}

int lualilka_audio_resume(lua_State* L) {
    if (audioTaskHandle != NULL && audioIsPaused) {
        AudioCommand cmd = {.type = AUDIO_CMD_RESUME, .gain = 0};
        xQueueSend(audioCommandQueue, &cmd, portMAX_DELAY);
    }
    return 0;
}

int lualilka_audio_set_volume(lua_State* L) {
    float gain = luaL_checknumber(L, 1);
    AudioCommand cmd = {.type = AUDIO_CMD_SET_GAIN, .gain = gain};
    if (audioTaskHandle != NULL && !audioIsFinished) {
        xQueueSend(audioCommandQueue, &cmd, portMAX_DELAY);
    }
    return 0;
}

int lualilka_audio_get_volume(lua_State* L) {
    lua_pushnumber(L, audioGain);
    return 1;
}

int lualilka_audio_is_playing(lua_State* L) {
    lua_pushboolean(L, audioIsPlaying && !audioIsFinished && !audioIsPaused);
    return 1;
}

static const luaL_Reg lualilka_audio[] = {
    {"play", lualilka_audio_play},
    {"stop", lualilka_audio_stop},
    {"pause", lualilka_audio_pause},
    {"resume", lualilka_audio_resume},
    {"set_volume", lualilka_audio_set_volume},
    {"get_volume", lualilka_audio_get_volume},
    {"is_playing", lualilka_audio_is_playing},
    {NULL, NULL},
};

int lualilka_audio_register(lua_State* L) {
    luaL_newlib(L, lualilka_audio);
    lua_setglobal(L, "audio");
    return 0;
}

void lualilka_audio_cleanup() {
    stopTaskAndClean();
    if (audioCommandQueue != NULL) {
        vQueueDelete(audioCommandQueue);
        audioCommandQueue = NULL;
    }
    if (audioMutex != NULL) {
        vSemaphoreDelete(audioMutex);
        audioMutex = NULL;
    }
    audioIsPlaying = false;
    audioIsPaused = false;
    audioIsFinished = false;
    audioGain = 1.0f;
}
