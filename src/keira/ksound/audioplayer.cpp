#include "keira/ksystem.h"
#include <AudioGeneratorMOD.h>
#include <AudioGeneratorWAV.h>
#include <AudioGeneratorMP3.h>
#include <AudioGeneratorAAC.h>
#include <AudioGeneratorFLAC.h>
#include <AudioOutputI2S.h>

#include "keira/mutex.h"

#include <lilka/config.h>
#include <lilka/audio.h>
#include "audioplayer.h"

namespace lilka {

AudioPlayer::~AudioPlayer() {
    if (K_PTHREAD_CHECK(audioPlayerThread)) audioPlayerThread->stop();
    vSemaphoreDelete(mutex);
}

AudioPlayer::AudioPlayer() {
}

AudioGenerator* AudioPlayer::createGenerator(const char* type) {
    if (strcmp(type, "mod") == 0) {
        return new AudioGeneratorMOD();
    } else if (strcmp(type, "wav") == 0) {
        return new AudioGeneratorWAV();
    } else if (strcmp(type, "mp3") == 0) {
        return new AudioGeneratorMP3();
    } else if (strcmp(type, "aac") == 0) {
        return new AudioGeneratorAAC();
    } else if (strcmp(type, "flac") == 0) {
        return new AudioGeneratorFLAC();
    }
    return nullptr;
}

bool AudioPlayer::play(lilka::Sound* sound, AudioOutput* customOutput) {
    // Stop any currently playing audio
    stopInternal();

    // Create command queue and mutex if not yet created
    if (commandQueue == nullptr) {
        commandQueue = xQueueCreate(8, sizeof(AudioCommand));
    } else {
        xQueueReset(commandQueue);
    }
    if (mutex == nullptr) {
        mutex = xSemaphoreCreateMutex();
    }

    // Create generator based on sound type
    generator = createGenerator(sound->type);
    if (!generator) {
        return false;
    }

    // Set up output
    if (customOutput) {
        output = customOutput;
        ownsOutput = false;
    } else {
        AudioOutputI2S* i2s = new AudioOutputI2S();
        i2s->SetPinout(LILKA_I2S_BCLK, LILKA_I2S_LRCK, LILKA_I2S_DOUT);
        output = i2s;
        ownsOutput = true;
    }

    // Create source from sound data
    source = new AudioFileSourcePROGMEM(sound->data, sound->size);

    // Begin playback
    generator->begin(source, output);

    // Set initial gain from system volume
    if (gain < 0) {
        gain = lilka::audio.getVolume() / 100.0f;
    }
    output->SetGain(gain);

    // Reset state
    playingSound = sound;
    playing = true;
    paused = false;
    finished = false;

    // Start background task on core 0
    audioPlayerThread = new KeiraThread(
        KT_CLBK_CAST(&AudioPlayer::audioTaskFunc), "AudioPlayer", 8192, KT_CLBK_DATA_CAST(this), KT_PRIO_IDLE, 0
    );
    ksystem.threads.spawn(audioPlayerThread);
    return true;
}

void AudioPlayer::audioTaskFunc() {
    while (1) {
        // Check for commands
        AudioCommand cmd;
        if (xQueueReceive(this->commandQueue, &cmd, 0) == pdTRUE) {
            switch (cmd.type) {
                case AUDIO_CMD_STOP:
                    if (this->generator) {
                        this->generator->stop();
                    }
                    KMTX_LOCK(this->mutex);
                    this->playing = false;
                    this->paused = false;
                    this->finished = true;
                    KMTX_UNLOCK(this->mutex);
                    // Suspend instead of self-delete — let stopInternal() delete us
                    // so the task stack is freed immediately by the caller
                    if (K_PTHREAD_CHECK(audioPlayerThread)) audioPlayerThread->suspend();
                    return;
                case AUDIO_CMD_PAUSE:
                    KMTX_LOCK(this->mutex);
                    this->paused = true;
                    KMTX_UNLOCK(this->mutex);
                    break;
                case AUDIO_CMD_RESUME:
                    KMTX_LOCK(this->mutex);
                    this->paused = false;
                    KMTX_UNLOCK(this->mutex);
                    break;
                case AUDIO_CMD_SET_GAIN:
                    KMTX_LOCK(this->mutex);
                    this->gain = cmd.gain;
                    if (this->gain < 0) this->gain = 0;
                    if (this->gain > 4) this->gain = 4;
                    KMTX_UNLOCK(this->mutex);
                    this->output->SetGain(this->gain);
                    break;
            }
        }

        KMTX_LOCK(this->mutex);
        bool currentPaused = this->paused;
        bool currentFinished = this->finished;
        KMTX_UNLOCK(this->mutex);

        if (currentFinished) {
            if (K_PTHREAD_CHECK(audioPlayerThread)) audioPlayerThread->suspend();
            return;
        }

        if (!currentPaused) {
            if (!this->generator->loop()) {
                // Track finished
                this->generator->stop();
                KMTX_LOCK(this->mutex);
                this->playing = false;
                this->finished = true;
                KMTX_UNLOCK(this->mutex);
                // Suspend instead of self-delete — let stopInternal() delete us
                if (K_PTHREAD_CHECK(audioPlayerThread)) audioPlayerThread->suspend();
                return;
            }
        }
        taskYIELD();
    }
}

void AudioPlayer::stopInternal() {
    // Stop the task if it's still running
    if (K_PTHREAD_CHECK(audioPlayerThread)) {
        if (!finished) {
            // Send stop command
            AudioCommand cmd = {.type = AUDIO_CMD_STOP, .gain = 0};
            xQueueSend(commandQueue, &cmd, portMAX_DELAY);
            // Wait for task to finish (up to 500ms)
            for (int i = 0; i < 50; i++) {
                if (finished) {
                    break;
                }
                vTaskDelay(pdMS_TO_TICKS(10));
            }
        }
        // Delete task from this context — memory is freed immediately
        // (unlike vTaskDelete(NULL) which defers to IDLE task)
        audioPlayerThread->stop();
        audioPlayerThread = nullptr;
    }

    // Always clean up resources (task may have finished naturally)
    if (generator) {
        delete generator;
        generator = nullptr;
    }
    if (source) {
        delete source;
        source = nullptr;
    }
    if (ownsOutput && output) {
        delete output;
    }
    output = nullptr;
    ownsOutput = false;
    playingSound = nullptr;
}

void AudioPlayer::stop() {
    stopInternal();
}

void AudioPlayer::pause() {
    if (K_PTHREAD_CHECK(audioPlayerThread) && playing && !paused) {
        AudioCommand cmd = {.type = AUDIO_CMD_PAUSE, .gain = 0};
        xQueueSend(commandQueue, &cmd, portMAX_DELAY);
    }
}

void AudioPlayer::resume() {
    if (K_PTHREAD_CHECK(audioPlayerThread) && paused) {
        AudioCommand cmd = {.type = AUDIO_CMD_RESUME, .gain = 0};
        xQueueSend(commandQueue, &cmd, portMAX_DELAY);
    }
}

void AudioPlayer::setGain(float gain) {
    AudioCommand cmd = {.type = AUDIO_CMD_SET_GAIN, .gain = gain};
    if (K_PTHREAD_CHECK(audioPlayerThread) && !finished) {
        xQueueSend(commandQueue, &cmd, portMAX_DELAY);
    }
}

float AudioPlayer::getGain() {
    return gain;
}

bool AudioPlayer::isPaused() {
    return paused;
}

bool AudioPlayer::isPlaying() {
    return playing && !finished && !paused;
}

bool AudioPlayer::isFinished() {
    return finished;
}

const lilka::Sound* AudioPlayer::getPlayingSound() {
    return playingSound;
}

void AudioPlayer::cleanup() {
    stopInternal();
    if (commandQueue != nullptr) {
        vQueueDelete(commandQueue);
        commandQueue = nullptr;
    }
    if (mutex != nullptr) {
        vSemaphoreDelete(mutex);
        mutex = nullptr;
    }
    playing = false;
    paused = false;
    finished = false;
    gain = -1.0f;
}

AudioPlayer audioPlayer;

} // namespace lilka