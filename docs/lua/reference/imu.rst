``imu`` - Акселерометр і гіроскоп
=================================

Робота з BMI260 на модулі 006 GYRO MAGNET (шина I2C, контакти 3 і 4). Див. також :doc:`modules` і :doc:`mag`.

Після ввімкнення BMI260 потребує конфігураційного блобу Bosch (8 КБ). За замовчуванням використовується вбудований;
його можна завантажити з файлу (``config = "bmi260.bin"``, шлях відносно скрипта) або не завантажувати (``config = false``).

.. note::

    Чи дає BMI260 коректні дані без блобу, ще не перевірено на залізі.

Приклад:

.. code-block:: lua

    local sensor = assert(imu.new())
    local d = sensor:read()
    print(d.ax, d.ay, d.az, d.gx, d.gy, d.gz)

Повний приклад: `LUA/modules/imu.lua <https://github.com/lilka-dev/examples/blob/main/LUA/modules/imu.lua>`_.

.. lua:autoclass:: imu
