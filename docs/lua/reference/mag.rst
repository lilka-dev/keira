``mag`` - Магнітометр
=====================

Робота з TMAG5273 на модулі 006 GYRO MAGNET (шина I2C, контакти 3 і 4). Див. також :doc:`modules` і :doc:`imu`.

Плата Лілки поруч спотворює поле (вимірювання ~100 мкТл замість 25-65 мкТл поля Землі), тому для компаса
потрібне калібрування: ``start_calibration()``, кілька секунд обертати Лілку в усіх напрямках, ``stop_calibration()``.
Зміщення можна зберегти через :doc:`state` і потім задати ``set_offset()``.

Приклад - компас:

.. code-block:: lua

    local sensor = assert(mag.new())
    local calibrating = false

    function lilka.update(delta)
        local state = controller.get_state()
        if state.a.just_pressed then
            calibrating = not calibrating
            if calibrating then sensor:start_calibration() else sensor:stop_calibration() end
        end
        if state.b.just_pressed then
            util.exit()
        end
    end

    function lilka.draw()
        local heading = sensor:heading()
        display.fill_screen(display.color565(0, 0, 0))
        display.set_cursor(10, 40)
        display.print(calibrating and "Обертайте Лілку...\nA - готово" or
            string.format("%.0f°\nA - калібрування", heading or 0))
    end

.. lua:autoclass:: mag
