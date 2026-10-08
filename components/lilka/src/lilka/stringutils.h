#pragma once
#include <cstddef>

namespace lilka {
/// Клас допоміжних функцій для роботи з текстом
class StringUtils {
public:
    /// Повертає вказівник на початок наступного Unicode-символу
    /// @param cstr вказівник на cString
    static char* uforward(char* cstr);

    /// Повертає вказівник на початок попереднього Unicode-символу
    /// @param cstr вказівник на cString
    static char* ubackward(char* cstr);

    /// Повертає довжину Unicode-символу в байтах
    /// @param cstr вказівник на cString
    static size_t uclen(char* cstr);

    /// Повертає довжину в байтах Unicode рядка
    /// @param cstr вказівник на cString
    /// @param cstr_end опціональний параметр кінця рядка
    static size_t ulen(char* cstr, const char* cstr_end = 0);
};

extern StringUtils sutils;
} // namespace lilka
