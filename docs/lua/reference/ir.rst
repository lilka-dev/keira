``ir`` - ІЧ-пульти
==================

Робота з модулем 002 IR: передавач KY-005 на контакті 4 (GPIO43) і приймач KY-022 на контакті 3 (GPIO44).
Розпізнає NEC, Samsung і Sony SIRC; будь-який інший сигнал можна прийняти й повторити як сирі тривалості.
Див. також :doc:`modules`.

Приклад:

.. code-block:: lua

    local remote = assert(ir.new())
    remote:send("NEC", 0x04, 0x08)

    local code = remote:receive() -- nil, якщо нічого не прийшло
    if code then
        print(code.protocol, code.address, code.command)
        remote:send_raw(code.raw) -- повторити як є
    end

Повний приклад: `LUA/modules/ir.lua <https://github.com/lilka-dev/examples/blob/main/LUA/modules/ir.lua>`_.

.. lua:autoclass:: ir
