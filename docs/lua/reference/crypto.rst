``crypto`` - Шифрування та хешування
-------------------------------------

Функції для шифрування, дешифрування та хешування даних.

AES шифрування використовує апаратне прискорення ESP32. CRC32 обчислюється за допомогою апаратної ROM-функції ESP32.

Приклад:

.. code-block:: lua
    :linenos:

    -- AES-256 шифрування та дешифрування
    local key = "01234567890abcdef01234567890abcd" -- 32 байти = AES-256
    local encrypted = crypto.encrypt("Привіт, світ!", key)
    console.print("Зашифровано:", encrypted)

    local decrypted = crypto.decrypt(encrypted, key)
    console.print("Розшифровано:", decrypted)

    -- MD5 хеш
    local hash = crypto.md5("hello world")
    console.print("MD5:", hash)

    -- CRC32 контрольна сума
    local checksum = crypto.crc32("hello world")
    console.print("CRC32:", checksum)

.. lua:autoclass:: crypto
