``nrf24`` - Радіо nRF24L01+
===========================

Робота з модулем 004 RF з nRF24L01+ (2.4 ГГц) на шині SPI2. Сумісний з бібліотекою RF24 для Arduino
за однакових каналу, швидкості, адрес і розміру пакета. Див. також :doc:`modules`.

Приклад:

.. code-block:: lua

    local radio = assert(nrf24.new({channel = 90}))
    radio:open_tx("1Node")
    radio:send("ping")

    local hits = radio:scan(2) -- 126 значень, індекс 1 = канал 0

Повний приклад: `LUA/modules/nrf24.lua <https://github.com/lilka-dev/examples/blob/main/LUA/modules/nrf24.lua>`_.

.. lua:autoclass:: nrf24
