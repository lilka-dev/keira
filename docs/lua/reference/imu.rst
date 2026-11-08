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

    function lilka.update(delta)
        if controller.get_state().b.just_pressed then
            util.exit()
        end
    end

    function lilka.draw()
        local d = sensor:read()
        display.fill_screen(display.color565(0, 0, 0))
        display.set_cursor(10, 40)
        if d then
            display.print(string.format("a: %.2f %.2f %.2f g\ng: %.1f %.1f %.1f", d.ax, d.ay, d.az, d.gx, d.gy, d.gz))
        end
    end

.. lua:autoclass:: imu
