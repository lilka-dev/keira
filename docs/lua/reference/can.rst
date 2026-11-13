``can`` - Шина CAN
==================

Робота з модулем 008 CAN через вбудований контролер TWAI ESP32-S3 (контакти 3 і 4 роз'єму). Див. також :doc:`modules`.

.. note::

    На чужій шині (наприклад, в автомобілі) почніть з ``mode = "listen"``: тоді Лілка нічого не передає
    і не підтверджує кадри. Перемичка J3 вмикає термінатор 120 Ом - замикайте її, лише якщо вузол на кінці шини.

Приклад:

.. code-block:: lua

    local bus = assert(can.new({bitrate = 500000, mode = "listen"}))
    local msg = bus:receive(100) -- чекати до 100 мс
    if msg then
        print(string.format("%03X", msg.id), #msg.data)
    end

Повний приклад: `LUA/modules/can.lua <https://github.com/lilka-dev/examples/blob/main/LUA/modules/can.lua>`_.

.. lua:autoclass:: can
