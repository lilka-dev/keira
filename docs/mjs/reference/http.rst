``http`` — HTTP-запити
----------------------

Функції для виконання HTTP/HTTPS-запитів.

.. note:: Для HTTPS-запитів використовується ``WiFiClientSecure`` без перевірки сертифікатів.

Приклад:

.. code-block:: javascript
    :linenos:

    wifi.connect("MyNetwork", "password");
    // Зачекати підключення...

    let result = http.execute({
        url: "https://api.example.com/data",
        method: "GET"
    });

    if (result.code === http.HTTP_CODE_OK) {
        console.print("Response:", result.response);
    }

    // Завантажити файл на SD-картку
    http.execute({
        url: "https://example.com/file.bin",
        file: "/downloads/file.bin"
    });

Константи
^^^^^^^^^

.. js:attribute:: http.HTTP_CODE_OK

    Код успішної відповіді (200).

Функції
^^^^^^^

.. js:function:: http.execute(opts)

    Виконує HTTP-запит.

    :param object opts: Об'єкт з параметрами:

        - ``url`` (``string``) — URL запиту (обов'язково).
        - ``method`` (``string``) — HTTP-метод (``"GET"``, ``"POST"`` тощо). За замовчуванням: ``"GET"`` (або ``"POST"``, якщо вказано ``body``).
        - ``body`` (``string``) — тіло запиту (необов'язково).
        - ``file`` (``string``) — шлях на SD-картці для збереження відповіді замість повернення тексту (необов'язково).

    :returns: Об'єкт ``{code, response}``:

        - ``code`` (``number``) — HTTP-код відповіді.
        - ``response`` (``string``) — тіло відповіді (тільки якщо ``file`` не вказано).
    :rtype: object
