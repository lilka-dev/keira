``console`` — Консоль
---------------------

Функції для вводу/виводу в Serial Monitor.

Приклад:

.. code-block:: javascript
    :linenos:

    console.print("Hello from mJS!");
    console.print("Value:", 42, true);

Функції
^^^^^^^

.. js:function:: console.print(...)

    Виводить значення у Serial Monitor. Приймає довільну кількість аргументів будь-яких типів (рядки, числа, булеві, об'єкти тощо). Аргументи розділяються табуляцією, в кінці додається перехід на новий рядок.
