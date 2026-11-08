``gps`` - Приймач GNSS
======================

Робота з модулем 005 GPS (u-blox NEO-M8N). Модуль надсилає NMEA-речення на 9600 бод; ``gps`` читає їх
з UART2 (контакти 3 і 4 роз'єму) і розбирає GGA, RMC, GSA та GSV. Див. також :doc:`modules`.

Приклад:

.. code-block:: lua

    local receiver = assert(gps.new())

    function lilka.update(delta)
        receiver:update()
        if controller.get_state().b.just_pressed then
            receiver:close()
            util.exit()
        end
    end

    function lilka.draw()
        local fix = receiver:fix()
        display.fill_screen(display.color565(0, 0, 0))
        display.set_cursor(10, 40)
        if fix.valid then
            display.print(string.format("%.5f %.5f\n%.0f m, %d sat", fix.latitude, fix.longitude,
                fix.altitude, fix.satellites))
        else
            display.print("Шукаю супутники: " .. #receiver:satellites())
        end
    end

.. lua:autoclass:: gps
