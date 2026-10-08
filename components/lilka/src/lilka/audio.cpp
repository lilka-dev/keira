#include "audio.h"
#include "config.h"
#include "ping.h"
#include "Preferences.h"
#include "serial.h"

namespace lilka {

Audio::Audio() {
}

// Thing to be run in parralel thread performing actual output
void welcomePlay(void* arg) {
#if LILKA_VERSION == 1
    serial.err("This part of code should never be called. Audio not supported for this version of lilka");
#elif LILKA_VERSION == 2
    // Signed 16-bit PCM
    const int16_t* ping = reinterpret_cast<const int16_t*>(ping_raw);
    auto volumeLevel = audio.getVolume();
    vTaskDelay(400 / portTICK_PERIOD_MS);

    int16_t buf;
    I2S.begin(I2S_PHILIPS_MODE, 22050, 16);
    for (int i = 0; i < ping_raw_size / 2; i++) {
        memcpy(&buf, &ping[i], 2);
        lilka::audio.adjustVolume(&buf, 2, 16, volumeLevel);

        I2S.write(buf >> 2);
        I2S.write(buf >> 2);
    }
    I2S.end();

    vTaskDelete(NULL);
#endif
}

void Audio::begin() {
    initPins();

    I2S.setAllPins(LILKA_I2S_BCLK, LILKA_I2S_LRCK, LILKA_I2S_DOUT, LILKA_I2S_DOUT, -1);

#ifndef LILKA_NO_AUDIO_HELLO
    if (getStartupSoundEnabled()) playStartupSound();
#endif
}

void Audio::playStartupSound() {
#if LILKA_VERSION == 2
    xTaskCreatePinnedToCore(welcomePlay, "welcomePlay", 4096, NULL, 1, NULL, 0);
#endif
}

void Audio::initPins() {
    // Set up I2S pins globally
    constexpr uint8_t pinCount = 3;
    uint8_t pins[pinCount] = {LILKA_I2S_BCLK, LILKA_I2S_LRCK, LILKA_I2S_DOUT};
    uint8_t funcs[pinCount] = {I2S0O_BCK_OUT_IDX, I2S0O_WS_OUT_IDX, I2S0O_SD_OUT_IDX};
    for (int i = 0; i < pinCount; i++) {
        gpio_pad_select_gpio(pins[i]);
        gpio_set_direction((gpio_num_t)pins[i], GPIO_MODE_OUTPUT);
        gpio_matrix_out(pins[i], funcs[i], false, false);
    }
}

void Audio::adjustVolume(void* buffer, size_t size, int bitsPerSample, uint32_t volumeLevel) {
    int gain = (volumeLevel * 1024.0) / 100.0;
    int samples = size / (bitsPerSample / 8);

    if (bitsPerSample == 8) {
        uint8_t* smp = static_cast<uint8_t*>(buffer);

        for (int i = 0; i < samples; i++) {
            *smp = (*smp * gain) >> 10;
            smp++;
        }
    } else if (bitsPerSample == 16) {
        int16_t* smp = static_cast<int16_t*>(buffer);

        for (int i = 0; i < samples; i++) {
            *smp = (*smp * gain) >> 10;
            smp++;
        }
    } else if (bitsPerSample == 32) {
        int32_t* smp = static_cast<int32_t*>(buffer);

        for (int i = 0; i < samples; i++) {
            *smp = (*smp * gain) >> 10;
            smp++;
        }
    }
}

// Getters/setters to work with NVS directly
// Single storage, less chance to create stupid problems with synchronising it's data
int Audio::getVolume() {
    Preferences prefs;

    prefs.begin(LILKA_SOUND_NVS_NAMESPACE, true);

    uint32_t volume = prefs.getUInt(LILKA_SOUND_NVS_VOLUME_LEVEL_KEY, LILKA_SOUND_NVS_DEFAULT_VOLUME);

    prefs.end();
    return volume;
}

void Audio::setVolume(int level) {
    Preferences prefs;

    prefs.begin(LILKA_SOUND_NVS_NAMESPACE, false);

    prefs.putUInt(LILKA_SOUND_NVS_VOLUME_LEVEL_KEY, level);

    prefs.end();
}

uint32_t Audio::getStartupSoundEnabled() {
    Preferences prefs;

    prefs.begin(LILKA_SOUND_NVS_NAMESPACE, true);
    bool startupSound = prefs.getBool(LILKA_SOUND_NVS_WELCOME_SOUND_KEY, LILKA_SOUND_NVS_DEFAULT_WELCOME_SOUND);

    prefs.end();
    return startupSound;
}

void Audio::setStartupSoundEnabled(bool enable) {
    Preferences prefs;

    prefs.begin(LILKA_SOUND_NVS_NAMESPACE, false);
    prefs.putBool(LILKA_SOUND_NVS_WELCOME_SOUND_KEY, enable);

    prefs.end();
}

Audio audio;

} // namespace lilka
