``spi`` - Шина SPI
=================

Функції для роботи з пристроями на користувацькій шині SPI через :ref:`роз'єм розширення <lilka-uk:extension>`.

.. note::

    Доступно лише на Лілці v2. Використовується окрема шина SPI2 (FSPI),
    тому робота з дисплеєм та SD-карткою не порушується.
    Лінією вибору пристрою (CS) потрібно керувати вручну через модуль ``gpio``.

Приклад:

.. code-block:: lua

    -- Читання ідентифікатора SPI Flash-пам'яті (команда 0x9F)

    cs = 21
    spi.begin(12, 13, 14) -- SCK = 12, MISO = 13, MOSI = 14
    gpio.set_mode(cs, gpio.OUTPUT)
    gpio.write(cs, gpio.HIGH)

    gpio.write(cs, gpio.LOW)
    local resp = spi.transfer({0x9F, 0x00, 0x00, 0x00})
    gpio.write(cs, gpio.HIGH)

    print(string.format("Manufacturer: 0x%02X", resp[2]))

.. lua:autoclass:: spi
