``gps`` - Приймач GNSS
======================

Робота з модулем 005 GPS (u-blox NEO-M8N). Модуль надсилає NMEA-речення на 9600 бод; ``gps`` читає їх
з UART2 (контакти 3 і 4 роз'єму) і розбирає GGA, RMC, GSA та GSV. Див. також :doc:`modules`.

Приклад:

.. code-block:: lua

    local receiver = assert(gps.new())

    function lilka.update(delta)
        receiver:update() -- читає й розбирає NMEA; викликайте часто
        local fix = receiver:fix()
        if fix.valid then
            print(fix.latitude, fix.longitude, fix.satellites)
        end
    end

Повний приклад: `LUA/modules/gps.lua <https://github.com/lilka-dev/examples/blob/main/LUA/modules/gps.lua>`_.

.. lua:autoclass:: gps
