``can`` - Шина CAN
==================

Робота з модулем 008 CAN через вбудований контролер TWAI ESP32-S3 (контакти 3 і 4 роз'єму). Див. також :doc:`modules`.

.. note::

    На чужій шині (наприклад, в автомобілі) почніть з ``mode = "listen"``: тоді Лілка нічого не передає
    і не підтверджує кадри. Перемичка J3 вмикає термінатор 120 Ом - замикайте її, лише якщо вузол на кінці шини.

Приклад - сніфер:

.. code-block:: lua

    local bus = assert(can.new({bitrate = 500000, mode = "listen"}))

    function lilka.update(delta)
        local msg = bus:receive()
        while msg do
            local hex = msg.data:gsub(".", function(c) return string.format("%02X ", c:byte()) end)
            console.print(string.format("%03X [%d] %s", msg.id, msg.dlc, hex))
            msg = bus:receive()
        end
        if controller.get_state().b.just_pressed then
            util.exit()
        end
    end

.. lua:autoclass:: can
