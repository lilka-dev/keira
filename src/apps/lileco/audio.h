#pragma once

#include <Arduino.h>
#include <AudioOutputI2S.h>
#include <freertos/FreeRTOS.h>
#include <memory>
#include <memory>

namespace lileco {

class ColecoAudio {
public:
    static ColecoAudio& instance();

    bool init();
    void stop();
    void setChannel(uint8_t channel, int frequency, int volume);

private:
    ColecoAudio() = default;
    void audioTask();
    static void audioTaskTrampoline(void* arg);
    void mixSamples(int16_t* buffer, size_t frames);

    static constexpr float SAMPLE_RATE = 22050.0f;
    static constexpr size_t CHANNEL_COUNT = 4;
    static constexpr size_t BUFFER_FRAMES = 256;

    struct ChannelParams {
        float phaseIncrement = 0.0f;
        float volume = 0.0f;
        bool isNoise = false;
    };

    ChannelParams channelParams[CHANNEL_COUNT];
    float channelPhases[CHANNEL_COUNT] = {0};
    float noiseAccumulator = 0.0f;
    float noiseState = 0.0f;
    uint32_t noiseLfsr = 0xACE1u;

    TaskHandle_t taskHandle = nullptr;
    bool running = false;
    portMUX_TYPE paramsMux = portMUX_INITIALIZER_UNLOCKED;

    std::unique_ptr<AudioOutputI2S> output;
};

} // namespace lileco
