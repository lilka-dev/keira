``i2c`` — Шина I2C
------------------

Функції для роботи з пристроями на шині I2C через роз'єм розширення.

Приклад:

.. code-block:: javascript
    :linenos:

    // Сканування шини I2C
    i2c.begin(8, 9); // SDA = 8, SCL = 9
    let devices = i2c.scan();
    for (let i = 0; i < devices.length; i++) {
        print("Знайдено пристрій: " + devices[i]);
    }

Функції
^^^^^^^

.. js:function:: i2c.begin(sda, scl, freq)

    Ініціалізує шину I2C.

    :param number sda: Номер піна SDA.
    :param number scl: Номер піна SCL.
    :param number freq: Частота шини в Гц (необов'язково, за замовчуванням ``100000``).

.. js:function:: i2c.set_clock(freq)

    Встановлює тактову частоту шини.

    :param number freq: Частота в Гц.

.. js:function:: i2c.scan()

    Сканує шину та повертає масив знайдених 7-бітних адрес.

    :return: Масив адрес.
    :rtype: number[]

.. js:function:: i2c.write(addr, data)

    Записує дані пристроєві.

    :param number addr: 7-бітна адреса пристрою.
    :param data: Один байт (число) або масив байтів.
    :return: Статус (``0`` — успіх).
    :rtype: number

.. js:function:: i2c.read(addr, count)

    Читає ``count`` байтів з пристрою.

    :param number addr: 7-бітна адреса пристрою.
    :param number count: Кількість байтів.
    :return: Масив прочитаних байтів.
    :rtype: number[]

.. js:function:: i2c.write_read(addr, wdata, count)

    Записує дані, а потім виконує читання з повторним стартом (repeated start).

    :param number addr: 7-бітна адреса пристрою.
    :param wdata: Один байт (число) або масив байтів для запису.
    :param number count: Кількість байтів для читання.
    :return: Масив прочитаних байтів.
    :rtype: number[]
