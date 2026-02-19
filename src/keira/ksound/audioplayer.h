#pragma once

#include <AudioOutput.h>
#include <AudioGenerator.h>
#include <AudioFileSourcePROGMEM.h>

#include "sound.h"

namespace lilka {

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

class AudioPlayer {
public:
    AudioPlayer();
    /// customOutput: NULL — створює власний AudioOutputI2S; інакше — caller відповідає за його життя.
    bool play(lilka::Sound* sound, AudioOutput* customOutput = nullptr);
    void stop();
    void pause();
    void resume();
    void setGain(float gain);
    float getGain();
    bool isPaused();
    bool isPlaying();
    bool isFinished();
    const lilka::Sound* getPlayingSound();
    void cleanup();

private:
    static AudioGenerator* createGenerator(const char* type);
    static void audioTaskFunc(void* arg);

    AudioGenerator* generator = nullptr;
    AudioFileSourcePROGMEM* source = nullptr;
    AudioOutput* output = nullptr;
    bool ownsOutput = false;
    lilka::Sound* playingSound = nullptr;

    TaskHandle_t taskHandle = nullptr;
    QueueHandle_t commandQueue = nullptr;
    SemaphoreHandle_t mutex = nullptr;

    volatile bool playing = false;
    volatile bool paused = false;
    volatile bool finished = false;
    float gain = -1.0f;

    void stopInternal();
};

extern AudioPlayer audioPlayer;

} // namespace lilka