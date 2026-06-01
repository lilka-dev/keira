``pwm`` - ШІМ (PWM)
==================

Функції для генерації ШІМ-сигналу на пінах :ref:`роз'єму розширення <lilka-uk:extension>`
за допомогою периферії ESP32 LEDC.

.. note::

    Системний бузер також використовує LEDC, тому радимо використовувати
    вищі номери каналів (наприклад, ``2``..``15``), щоб уникнути конфлікту.

Приклад:

.. code-block:: lua

    -- Плавна зміна яскравості світлодіода на піні 12

    led_pin = 12
    channel = 2

    pwm.setup(channel, 5000, 8) -- канал 2, 5 кГц, 8 біт
    pwm.attach_pin(led_pin, channel)

    while true do
        for duty = 0, 255 do
            pwm.write(channel, duty)
            util.sleep(0.005)
        end
        for duty = 255, 0, -1 do
            pwm.write(channel, duty)
            util.sleep(0.005)
        end
        if controller.get_state().a.just_pressed then
            break
        end
    end

.. lua:autoclass:: pwm
