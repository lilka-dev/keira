``i2c`` - Шина I2C
=================

Функції для роботи з пристроями на шині I2C через :ref:`роз'єм розширення <lilka-uk:extension>`.

Приклад:

.. code-block:: lua

    -- Сканування шини I2C та вивід знайдених адрес

    i2c.begin(8, 9) -- SDA = 8, SCL = 9
    local devices = i2c.scan()
    for _, addr in ipairs(devices) do
        print(string.format("Знайдено пристрій за адресою 0x%02X", addr))
    end

.. lua:autoclass:: i2c
