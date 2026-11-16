``lora`` - LoRa-радіо
=====================

Робота з модулем 003 LORA (SX1262 для 868/915 МГц або SX1268 для 433 МГц) на шині SPI2.
Див. також :doc:`modules`.

.. warning::

    Перевірте, які частоти й потужності дозволені у вашій країні, і не передавайте без антени.
    Обидві Лілки мають використовувати однакові ``frequency``, ``sf``, ``bandwidth``, ``cr`` і ``sync_word``.

Приклад:

.. code-block:: lua

    local radio = assert(lora.new({chip = "sx1262", frequency = 868100000, sf = 7}))
    radio:send("Привіт!")

    local packet = radio:receive() -- nil, якщо пакета немає
    if packet then
        print(packet.data, packet.rssi, packet.snr)
    end

Повний приклад: `LUA/modules/lora.lua <https://github.com/lilka-dev/examples/blob/main/LUA/modules/lora.lua>`_.

.. lua:autoclass:: lora
