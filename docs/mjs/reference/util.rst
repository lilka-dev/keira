``util`` — Утиліти
------------------

Допоміжні функції.

Функції
^^^^^^^

.. js:function:: util.time()

    Повертає час з моменту запуску пристрою у секундах (дробове число).

    :returns: Час у секундах.
    :rtype: number

.. js:function:: util.sleep(seconds)

    Призупиняє виконання програми на задану кількість секунд.

    :param number seconds: Кількість секунд (може бути дробовим).

.. js:function:: util.exit()

    Завершує виконання mJS-скрипта.

.. js:function:: util.free_ram()

    Повертає кількість вільної оперативної пам'яті у байтах.

    :returns: Вільна пам'ять.
    :rtype: number

.. js:function:: util.total_ram()

    Повертає загальний розмір оперативної пам'яті у байтах.

    :returns: Загальна пам'ять.
    :rtype: number
