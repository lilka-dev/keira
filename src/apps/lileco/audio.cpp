#include "audio.h"

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <lilka.h>

#include <AudioOutputI2S.h>
#include <memory>
#include <cstring>

namespace lileco {

ColecoAudio& ColecoAudio::instance() {
    static ColecoAudio audio;
    return audio;
}

bool ColecoAudio::init() {
    if (running) return true;

    lilka::audio.initPins();

    output.reset(new AudioOutputI2S());
    output->SetPinout(LILKA_I2S_BCLK, LILKA_I2S_LRCK, LILKA_I2S_DOUT);
    output->SetRate(static_cast<int>(SAMPLE_RATE));
    output->SetBitsPerSample(16);
    output->SetChannels(2);
    if (!output->begin()) {
        lilka::serial.err("ColecoAudio: failed to start AudioOutputI2S");
        output.reset();
        return false;
    }

    noiseAccumulator = 0.0f;
    noiseState = 0.0f;
    noiseLfsr = 0xACE1u;
    memset(channelParams, 0, sizeof(channelParams));
    memset(channelPhases, 0, sizeof(channelPhases));

    running = true;
    BaseType_t res = xTaskCreatePinnedToCore(
        audioTaskTrampoline,
        "coleco_audio",
        4096,
        this,
        1,
        &taskHandle,
        1
    );
    if (res != pdPASS) {
        lilka::serial.err("ColecoAudio: failed to start audio task");
        running = false;
        output->stop();
        output.reset();
        return false;
    }
    return true;
}

void ColecoAudio::stop() {
    if (!running) return;
    running = false;
    while (taskHandle != nullptr) {
        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
    if (output) {
        output->stop();
        output.reset();
    }
}

void ColecoAudio::setChannel(uint8_t channel, int frequency, int volume) {
    if (channel >= CHANNEL_COUNT) return;
    float phaseIncrement = 0.0f;
    if (frequency > 0) {
        phaseIncrement = static_cast<float>(frequency) / SAMPLE_RATE;
    }
    float vol = static_cast<float>(volume) / 240.0f;
    if (vol < 0.0f) vol = 0.0f;
    if (vol > 1.0f) vol = 1.0f;

    portENTER_CRITICAL(&paramsMux);
    channelParams[channel].phaseIncrement = phaseIncrement;
    channelParams[channel].volume = vol;
    channelParams[channel].isNoise = (channel >= 3);
    portEXIT_CRITICAL(&paramsMux);
}

void ColecoAudio::audioTaskTrampoline(void* arg) {
    static_cast<ColecoAudio*>(arg)->audioTask();
}

void ColecoAudio::audioTask() {
    int16_t buffer[BUFFER_FRAMES * 2];
    int16_t samplePair[2];
    while (running) {
        mixSamples(buffer, BUFFER_FRAMES);
        lilka::audio.adjustVolume(buffer, sizeof(buffer), 16);
        if (!output) continue;
        for (size_t i = 0; i < BUFFER_FRAMES; i++) {
            samplePair[0] = buffer[i * 2];
            samplePair[1] = buffer[i * 2 + 1];
            while (running && output && !output->ConsumeSample(samplePair)) {
                vTaskDelay(1);
            }
        }
    }
    taskHandle = nullptr;
    vTaskDelete(NULL);
}

void ColecoAudio::mixSamples(int16_t* buffer, size_t frames) {
    ChannelParams local[CHANNEL_COUNT];
    portENTER_CRITICAL(&paramsMux);
    memcpy(local, channelParams, sizeof(local));
    portEXIT_CRITICAL(&paramsMux);

    for (size_t i = 0; i < frames; i++) {
        float sample = 0.0f;
        for (size_t ch = 0; ch < CHANNEL_COUNT; ch++) {
            const auto& param = local[ch];
            if (param.volume <= 0.0f || param.phaseIncrement <= 0.0f) continue;
            float value = 0.0f;
            if (param.isNoise) {
                noiseAccumulator += param.phaseIncrement;
                if (noiseAccumulator >= 1.0f) {
                    noiseAccumulator -= 1.0f;
                    noiseState = (noiseLfsr & 1) ? 1.0f : -1.0f;
                    uint32_t newBit = ((noiseLfsr >> 0) ^ (noiseLfsr >> 1)) & 1;
                    noiseLfsr = (noiseLfsr >> 1) | (newBit << 15);
                }
                value = noiseState;
            } else {
                channelPhases[ch] += param.phaseIncrement;
                if (channelPhases[ch] >= 1.0f) channelPhases[ch] -= 1.0f;
                value = channelPhases[ch] < 0.5f ? 1.0f : -1.0f;
            }
            sample += value * param.volume;
        }
        sample *= 5000.0f;
        if (sample > 32767.0f) sample = 32767.0f;
        if (sample < -32767.0f) sample = -32767.0f;
        int16_t pcm = static_cast<int16_t>(sample);
        buffer[i * 2] = pcm;
        buffer[i * 2 + 1] = pcm;
    }
}

} // namespace lileco
