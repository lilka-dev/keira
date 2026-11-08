``lora`` - LoRa-радіо
=====================

Робота з модулем 003 LORA (SX1262 для 868/915 МГц або SX1268 для 433 МГц) на шині SPI2.
Див. також :doc:`modules`.

.. warning::

    Перевірте, які частоти й потужності дозволені у вашій країні, і не передавайте без антени.
    Обидві Лілки мають використовувати однакові ``frequency``, ``sf``, ``bandwidth``, ``cr`` і ``sync_word``.

Приклад - чат між двома Лілками:

.. code-block:: lua

    local radio = assert(lora.new({chip = "sx1262", frequency = 868100000, sf = 7}))
    local last = "-"

    function lilka.update(delta)
        local packet = radio:receive()
        if packet then
            last = string.format("%s (%.0f dBm, SNR %.1f)", packet.data, packet.rssi, packet.snr)
        end
        local state = controller.get_state()
        if state.a.just_pressed then
            radio:send("Привіт від Лілки!")
        end
        if state.b.just_pressed then
            util.exit()
        end
    end

    function lilka.draw()
        display.fill_screen(display.color565(0, 0, 0))
        display.set_cursor(10, 40)
        display.print("A - надіслати\n" .. last)
    end

.. lua:autoclass:: lora
