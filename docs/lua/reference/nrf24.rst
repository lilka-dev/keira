``nrf24`` - Радіо nRF24L01+
===========================

Робота з модулем 004 RF з nRF24L01+ (2.4 ГГц) на шині SPI2. Сумісний з бібліотекою RF24 для Arduino
за однакових каналу, швидкості, адрес і розміру пакета. Див. також :doc:`modules`.

Приклад - сканер діапазону 2.4 ГГц:

.. code-block:: lua

    local radio = assert(nrf24.new())
    local hits = {}

    function lilka.update(delta)
        hits = radio:scan(2)
        if controller.get_state().b.just_pressed then
            util.exit()
        end
    end

    function lilka.draw()
        display.fill_screen(display.color565(0, 0, 0))
        for ch = 1, #hits do
            local h = hits[ch] * 40
            display.fill_rect(10 + ch * 2, 200 - h, 2, h, display.color565(0, 255, 0))
        end
    end

.. lua:autoclass:: nrf24
