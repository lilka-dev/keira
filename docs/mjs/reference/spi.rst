``spi`` — Шина SPI
------------------

Функції для роботи з пристроями на користувацькій шині SPI через роз'єм розширення.

.. note::

    Доступно лише на Лілці v2. Використовується окрема шина SPI2 (FSPI),
    тому робота з дисплеєм та SD-карткою не порушується.
    Лінією вибору пристрою (CS) потрібно керувати вручну через модуль ``gpio``.

Приклад:

.. code-block:: javascript
    :linenos:

    let cs = 21;
    spi.begin(12, 13, 14); // SCK, MISO, MOSI
    gpio.mode(cs, gpio.OUTPUT);
    gpio.write(cs, gpio.HIGH);

    gpio.write(cs, gpio.LOW);
    let resp = spi.transfer([0x9F, 0x00, 0x00, 0x00]);
    gpio.write(cs, gpio.HIGH);
    print("Manufacturer: " + resp[1]);

Константи
^^^^^^^^^

.. js:attribute:: spi.MODE0

    Режим SPI 0.

.. js:attribute:: spi.MODE1

    Режим SPI 1.

.. js:attribute:: spi.MODE2

    Режим SPI 2.

.. js:attribute:: spi.MODE3

    Режим SPI 3.

Функції
^^^^^^^

.. js:function:: spi.begin(sck, miso, mosi)

    Ініціалізує шину SPI.

    :param number sck: Номер піна SCK (тактовий сигнал).
    :param number miso: Номер піна MISO.
    :param number mosi: Номер піна MOSI.

.. js:function:: spi.transfer(data, frequency, mode)

    Передає дані по шині SPI (повний дуплекс) і повертає прийняті дані.

    :param data: Один байт (число) або масив байтів.
    :param number frequency: Частота в Гц (необов'язково, за замовчуванням ``4000000``).
    :param number mode: Режим SPI (необов'язково, за замовчуванням ``spi.MODE0``).
    :return: Прийнятий байт (число) або масив байтів.

.. js:function:: spi.close()

    Завершує роботу з шиною SPI.
