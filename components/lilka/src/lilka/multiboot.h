#ifndef LILKA_MULTIBOOT_H
#define LILKA_MULTIBOOT_H
#include <esp_ota_ops.h>
#include <Arduino.h>
#ifndef MULTIBOOT_CMD_LEN
#    define MULTIBOOT_CMD_LEN 1024
#endif
#include "fileutils.h"

namespace lilka {

/// Завантажувач прошивок з microSD-картки.
///
/// Дозволяє прочитати файл прошивки з microSD-картки в OTA-розділ та запустити його один раз,
/// не замінюючи поточну прошивку.
///
/// Нова прошивка зберігається в OTA-розділі та запускається при перезавантаженні,
/// але активною залишається основна прошивка (app rollback).
///
/// Це дозволяє компілювати за записувати різні прошивки на microSD-карту і запускати їх через браузер SD-карти без необхідності перепрошивки.
///
/// Детальніше про auto-rollback: https://docs.espressif.com/projects/esp-idf/en/stable/esp32/api-reference/system/ota.html#rollback-process
///
/// Приклад використання:
///
/// @code
/// #include <lilka.h>
///
/// void setup() {
///     lilka::begin();
///
///     Serial.println("Завантаження прошивки 'firmware.bin' з microSD-картки...");
///
///     lilka::multiboot.start("/sd/firmware.bin"); // Почати завантаження
///     while (lilka::multiboot.process() != 0) { // Обробити завантаження
///         Serial.println("Завантажено " + String(lilka::multiboot.getBytesWritten()) + "/" + String(lilka::multiboot.getBytesTotal()) + " байтів");
///     }
///     lilka::multiboot.finishAndReboot(); // Завершити завантаження та перезавантажити пристрій
/// }
/// @endcode
class MultiBoot {
public:
    MultiBoot();
    /// Ініціалізувати завантажувач.
    /// \warning Цей метод викликається автоматично при виклику `lilka::begin()`.
    void begin();
    /// Почати завантаження.
    /// \return 0, якщо завантаження почалося успішно, <0 - у разі помилки.
    int start(String path);
    /// Обробити завантаження.
    /// Цей метод повинен викликатися в циклі, поки не поверне 0.
    /// Щоразу він опрацьовує частину файлу та записує її в OTA-розділ.
    /// \return 0, якщо завантаження завершилося успішно, <0 - у разі помилки, >0 - означає кількість байтів, які було оброблено.
    int process();
    /// Перервати завантаження.
    void cancel();
    /// Отримати кількість байтів, які було записано в OTA-розділ.
    /// \return Кількість байтів
    int getBytesWritten();
    /// Отримати загальну кількість байтів, які потрібно записати в OTA-розділ.
    /// \return Кількість байтів
    int getBytesTotal();
    /// Завершити завантаження та перезавантажити пристрій.
    /// \return <0 - у разі помилки. В разі успіху цей метод не повертається, оскільки пристрій перезавантажується.
    int finishAndReboot();
    /// Завантажити останню завантажену прошивку
    void bootLast();
    /// Встановити параметри командного рядку. Повний шлях до файлу прошивки передати першим параметром
    /// @param cmd
    void setCMDParams(String cmd);
    /// Повертає кількість аргументів командного рядка.
    /// @return Кількість аргументів (argc), включно з повним шляхом до файлу прошивки.
    int getArgc();
    /// Повертає масив аргументів командного рядка.
    /// @return Вказівник на масив рядків (argv), де кожен елемент — окремий аргумент.
    char** getArgv();
    /// Повертає причину останнього скидання пристрою.
    /// @return Причина скидання (esp_reset_reason_t).
    esp_reset_reason_t getResetReason();
    /// Отримати шлях до файлу прошивки, встановлений у параметрах командного рядка.
    /// @return Шлях до файлу прошивки як рядок (String). Якщо шлях
    String getFirmwarePath();

private:
    String path;
    FILE* file;
    esp_ota_handle_t ota_handle;
    const esp_partition_t* current_partition;
    const esp_partition_t* ota_partition;
    int bytesWritten;
    int bytesTotal;
    int argc = 0;
    char** argv = NULL;
};

/// Екземпляр класу `MultiBoot`, який можна використовувати для роботи з завантажувачем.
/// Вам не потрібно інстанціювати `MultiBoot` вручну.
extern MultiBoot multiboot;

} // namespace lilka

#endif // LILKA_MULTIBOOT_H
