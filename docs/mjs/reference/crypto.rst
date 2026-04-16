``crypto`` — Криптографія
-------------------------

Функції для шифрування, хешування та обчислення контрольних сум.

Приклад:

.. code-block:: javascript
    :linenos:

    // AES шифрування/дешифрування
    let key = "0123456789abcdef"; // 16 байтів для AES-128
    let encrypted = crypto.encrypt("Hello, world!", key);
    let decrypted = crypto.decrypt(encrypted, key);
    console.print(decrypted); // "Hello, world!"

    // MD5 хеш
    let hash = crypto.md5("test");
    console.print(hash);

    // CRC32
    let crc = crypto.crc32("test data");
    console.print(crc);

Функції
^^^^^^^

.. js:function:: crypto.encrypt(plaintext, key)

    Шифрує текст алгоритмом AES-CBC з PKCS7-доповненням та випадковим IV.

    :param string plaintext: Текст для шифрування.
    :param string key: Ключ шифрування (16, 24 або 32 байти для AES-128/192/256).
    :returns: Зашифрований текст у hex-форматі (IV + шифротекст) або ``undefined`` при помилці.
    :rtype: string

.. js:function:: crypto.decrypt(hex_string, key)

    Розшифровує текст, зашифрований ``crypto.encrypt()``.

    :param string hex_string: Зашифрований текст у hex-форматі.
    :param string key: Ключ шифрування (16, 24 або 32 байти).
    :returns: Розшифрований текст або ``undefined`` при помилці.
    :rtype: string

.. js:function:: crypto.md5(data)

    Обчислює MD5-хеш рядка.

    :param string data: Вхідні дані.
    :returns: MD5-хеш у hex-форматі (32 символи).
    :rtype: string

.. js:function:: crypto.crc32(data)

    Обчислює контрольну суму CRC32.

    :param string data: Вхідні дані.
    :returns: CRC32 значення.
    :rtype: number
