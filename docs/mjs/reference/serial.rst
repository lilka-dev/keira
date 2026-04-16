``serial`` — Послідовний порт
-----------------------------

Функції для роботи з апаратним послідовним портом (UART). Використовує ``HardwareSerial(1)`` на пінах P4 (RX) / P3 (TX).

Приклад:

.. code-block:: javascript
    :linenos:

    serial.begin(9600);
    serial.println("Hello from mJS!");

    while (serial.available() > 0) {
        let data = serial.read(64);
        console.print("Received:", data);
    }

    serial.end();

Константи
^^^^^^^^^

На об'єкті ``serial`` доступні константи конфігурації порту: ``SERIAL_5N1``, ``SERIAL_6N1``, ``SERIAL_7N1``, ``SERIAL_8N1``, ``SERIAL_5N2``, ``SERIAL_6N2``, ``SERIAL_7N2``, ``SERIAL_8N2``, ``SERIAL_5E1``, ``SERIAL_6E1``, ``SERIAL_7E1``, ``SERIAL_8E1``, ``SERIAL_5E2``, ``SERIAL_6E2``, ``SERIAL_7E2``, ``SERIAL_8E2``, ``SERIAL_5O1``, ``SERIAL_6O1``, ``SERIAL_7O1``, ``SERIAL_8O1``, ``SERIAL_5O2``, ``SERIAL_6O2``, ``SERIAL_7O2``, ``SERIAL_8O2``.

Функції
^^^^^^^

.. js:function:: serial.begin([baud[, config]])

    Відкриває послідовний порт.

    :param number baud: *(необов'язково)* Швидкість (за замовчуванням: 115200).
    :param number config: *(необов'язково)* Конфігурація (за замовчуванням: ``SERIAL_8N1``).

.. js:function:: serial.end()

    Закриває послідовний порт.

.. js:function:: serial.available()

    Повертає кількість байтів, доступних для читання.

    :returns: Кількість байтів.
    :rtype: number

.. js:function:: serial.flush()

    Очищує буфер виводу.

.. js:function:: serial.peek()

    Повертає наступний байт без його видалення з буфера.

    :returns: Значення байта.
    :rtype: number

.. js:function:: serial.print(...)

    Виводить значення в послідовний порт (без перенесення рядка).

.. js:function:: serial.println(...)

    Виводить значення в послідовний порт з перенесенням рядка.

.. js:function:: serial.read([bytes])

    Зчитує дані з послідовного порта.

    - Без аргументів: зчитує один байт і повертає ``number``.
    - З ``bytes``: зчитує до ``bytes`` байтів і повертає ``string``.

.. js:function:: serial.setTimeout(timeout)

    Встановлює таймаут читання.

    :param number timeout: Таймаут у мілісекундах.

.. js:function:: serial.write(...)

    Записує дані у послідовний порт. Приймає рядки або числа.
