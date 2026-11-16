``mag`` - Магнітометр
=====================

Робота з TMAG5273 на модулі 006 GYRO MAGNET (шина I2C, контакти 3 і 4). Див. також :doc:`modules` і :doc:`imu`.

Плата Лілки поруч спотворює поле (вимірювання ~100 мкТл замість 25-65 мкТл поля Землі), тому для компаса
потрібне калібрування: ``start_calibration()``, кілька секунд обертати Лілку в усіх напрямках, ``stop_calibration()``.
Зміщення можна зберегти через :doc:`state` і потім задати ``set_offset()``.

Приклад:

.. code-block:: lua

    local sensor = assert(mag.new())
    sensor:start_calibration()
    -- ... обертайте Лілку, викликаючи sensor:read() ...
    sensor:stop_calibration()
    print(sensor:heading())

Повний приклад: `LUA/modules/mag.lua <https://github.com/lilka-dev/examples/blob/main/LUA/modules/mag.lua>`_.

.. lua:autoclass:: mag
