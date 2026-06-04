``pwm`` — ШІМ (PWM)
-------------------

Функції для генерації ШІМ-сигналу за допомогою периферії ESP32 LEDC.

.. note::

    Системний бузер також використовує LEDC, тому радимо використовувати
    вищі номери каналів (наприклад, ``2``..``15``), щоб уникнути конфлікту.

Приклад:

.. code-block:: javascript
    :linenos:

    let led_pin = 12;
    let channel = 2;
    pwm.setup(channel, 5000, 8); // канал 2, 5 кГц, 8 біт
    pwm.attach_pin(led_pin, channel);
    pwm.write(channel, 128); // 50% заповнення

Функції
^^^^^^^

.. js:function:: pwm.setup(channel, freq, resolution_bits)

    Налаштовує канал ШІМ.

    :param number channel: Номер каналу (``0``..``15``).
    :param number freq: Частота в Гц.
    :param number resolution_bits: Розрядність (``1``..``16``, необов'язково, за замовчуванням ``8``).
    :return: Фактична встановлена частота в Гц.
    :rtype: number

.. js:function:: pwm.attach_pin(pin, channel)

    Прив'язує GPIO-пін до каналу ШІМ.

    :param number pin: Номер піна.
    :param number channel: Номер каналу.

.. js:function:: pwm.detach_pin(pin)

    Відв'язує GPIO-пін від каналу ШІМ.

    :param number pin: Номер піна.

.. js:function:: pwm.write(channel, duty)

    Встановлює коефіцієнт заповнення (duty) для каналу.

    :param number channel: Номер каналу.
    :param number duty: Значення заповнення (``0``..``2^resolution - 1``).

.. js:function:: pwm.write_tone(channel, freq)

    Генерує тон заданої частоти на каналі.

    :param number channel: Номер каналу.
    :param number freq: Частота в Гц.
    :return: Фактична встановлена частота в Гц.
    :rtype: number

.. js:function:: pwm.read(channel)

    Читає поточне значення заповнення (duty) каналу.

    :param number channel: Номер каналу.
    :return: Поточне значення заповнення.
    :rtype: number
