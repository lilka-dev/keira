#pragma once

#include <cstdint>
#include <cstddef>

namespace lilka {

/// Звук
///
/// Містить дані аудіо-файлу та його тип.
/// Дані зберігаються в пам'яті у вигляді масиву байтів (наприклад, вміст .mod, .wav, .mp3-файлу).
///
/// @note Аналог Image, але для звукових даних.
class Sound {
public:
    /// Створити звук із заданим буфером даних та типом.
    ///
    /// @param data Вказівник на масив байтів аудіо-файлу. Sound бере на себе відповідальність за звільнення пам'яті.
    /// @param size Розмір даних у байтах.
    /// @param type Тип аудіо: "mod", "wav", "mp3", "aac", "flac".
    Sound(uint8_t* data, size_t size, const char* type);
    ~Sound();

    /// Дані аудіо-файлу.
    uint8_t* data;

    /// Розмір даних у байтах.
    size_t size;

    /// Тип аудіо-файлу ("mod", "wav", "mp3", "aac", "flac").
    char type[8];
};

} // namespace lilka
