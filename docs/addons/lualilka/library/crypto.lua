---@meta

---@class crypto
crypto = {}

---Шифрує рядок за допомогою AES-CBC з PKCS7-доповненням.
---
---Ключ має бути рівно 16, 24 або 32 байти (AES-128, AES-192 або AES-256 відповідно).
---
---Повертає hex-рядок, що містить випадковий вектор ініціалізації (IV) та зашифровані дані.
---Використовує апаратне прискорення AES ESP32.
---
---@param plaintext string текст для шифрування
---@param key string ключ шифрування (16, 24 або 32 байти)
---@return string hex_encoded зашифровані дані у форматі hex (IV + шифротекст)
---@usage
--- local encrypted = crypto.encrypt("Привіт, світ!", "0123456789abcdef")
--- console.print(encrypted) -- hex-рядок
function crypto.encrypt(plaintext, key) end

---Дешифрує hex-рядок, зашифрований функцією crypto.encrypt.
---
---Ключ має бути тим самим, що використовувався для шифрування.
---
---@param hex_encrypted string зашифровані дані у форматі hex (результат crypto.encrypt)
---@param key string ключ дешифрування (16, 24 або 32 байти)
---@return string plaintext розшифрований текст
---@usage
--- local encrypted = crypto.encrypt("Привіт, світ!", "0123456789abcdef")
--- local decrypted = crypto.decrypt(encrypted, "0123456789abcdef")
--- console.print(decrypted) -- "Привіт, світ!"
function crypto.decrypt(hex_encrypted, key) end

---Обчислює MD5-хеш рядка.
---
---Повертає hex-рядок довжиною 32 символи.
---
---@param data string дані для хешування
---@return string hex_hash MD5-хеш у форматі hex (32 символи)
---@usage
--- local hash = crypto.md5("Привіт, світ!")
--- console.print(hash) -- "d41d8cd98f00b204e9800998ecf8427e" (приклад)
function crypto.md5(data) end

---Обчислює CRC32 контрольну суму рядка.
---
---Використовує апаратну ROM-функцію ESP32.
---
---@param data string дані для обчислення контрольної суми
---@return integer crc32 контрольна сума CRC32
---@usage
--- local checksum = crypto.crc32("hello")
--- console.print(checksum)
function crypto.crc32(data) end

return crypto
