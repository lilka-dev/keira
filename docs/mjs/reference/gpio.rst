``gpio`` — Керування GPIO-пінами
---------------------------------

Функції для роботи з GPIO-пінами роз'єму розширення.

Приклад:

.. code-block:: javascript
    :linenos:

    // Блимаємо світлодіодом на піні 12
    let led_pin = 12;
    gpio.mode(led_pin, gpio.OUTPUT);

    while (true) {
        gpio.write(led_pin, gpio.HIGH);
        util.sleep(0.5);
        gpio.write(led_pin, gpio.LOW);
        util.sleep(0.5);

        if (controller.get_state().a.just_pressed) {
            break;
        }
    }

Константи
^^^^^^^^^

.. js:attribute:: gpio.INPUT

    Режим входу.

.. js:attribute:: gpio.OUTPUT

    Режим виходу.

.. js:attribute:: gpio.INPUT_PULLUP

    Режим входу з підтяжкою вгору.

.. js:attribute:: gpio.INPUT_PULLDOWN

    Режим входу з підтяжкою вниз.

.. js:attribute:: gpio.HIGH

    Високий рівень сигналу.

.. js:attribute:: gpio.LOW

    Низький рівень сигналу.

Функції
^^^^^^^

.. js:function:: gpio.mode(pin, mode)

    Встановлює режим GPIO-піна.

    :param number pin: Номер піна.
    :param number mode: Режим (``gpio.INPUT``, ``gpio.OUTPUT``, тощо).

.. js:function:: gpio.read(pin)

    Зчитує цифрове значення з піна.

    :param number pin: Номер піна.
    :returns: ``gpio.HIGH`` або ``gpio.LOW``.
    :rtype: number

.. js:function:: gpio.write(pin, value)

    Записує цифрове значення на пін.

    :param number pin: Номер піна.
    :param number value: ``gpio.HIGH`` або ``gpio.LOW``.

.. js:function:: gpio.analog_read(pin)

    Зчитує аналогове значення з піна (0–4095).

    :param number pin: Номер піна.
    :returns: Аналогове значення.
    :rtype: number
