#pragma once

#include <I2S.h>

#define LILKA_SOUND_NVS_NAMESPACE             "sound"
#define LILKA_SOUND_NVS_VOLUME_LEVEL_KEY      "volumeLevel"
#define LILKA_SOUND_NVS_WELCOME_SOUND_KEY     "startupSound"

#define LILKA_SOUND_NVS_DEFAULT_VOLUME        100
#define LILKA_SOUND_NVS_DEFAULT_WELCOME_SOUND true

namespace lilka {

/// Клас для ініціалізації аудіо.
///
/// Цей клас лише встановлює піни для I2S і відтворює тестовий звук.
///
/// Для роботи з аудіо використовуйте клас I2S напряму: https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/i2s.html#sample-code
class Audio {
public:
    Audio();
    /// Налаштоувує піни для I2S і відтворює тестовий звук.
    /// \warning Цей метод викликається автоматично при виклику `lilka::begin()`.
    static void begin();
    /// Відтворює звук вітання, якщо він увімкнений в налаштуваннях.
    static void playStartupSound();
    /// Налаштувує піни для I2S.
    /// Цей метод варто викликати перед викликом `i2s_driver_install()`.
    static void initPins();
    /// Регулює гучність до рівня, збереженого в налаштуваннях
    /// Цей метод бажано викликати перед `i2s_write`.
    static void adjustVolume(void* buffer, size_t size, int bitsPerSample, uint32_t volumeLevel);
    /// Повертає рівень гучності
    static int getVolume();
    /// Встановлює рівень гучності
    static void setVolume(int level);
    /// Перевіряє чи увімкнено звук вітання
    static uint32_t getStartupSoundEnabled();
    /// Вмикає чи вимикає звук вітання
    static void setStartupSoundEnabled(bool enable);
};

extern Audio audio;

} // namespace lilka
