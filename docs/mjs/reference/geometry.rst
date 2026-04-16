``geometry`` — Геометрія
------------------------

Функції для роботи з геометричними примітивами (перетин ліній, AABB тощо).

Приклад:

.. code-block:: javascript
    :linenos:

    // Перевірка перетину двох ліній
    let hit = geometry.intersect_lines(0, 0, 100, 100, 100, 0, 0, 100);

    // Перевірка перетину двох прямокутників
    let overlap = geometry.intersect_aabb(0, 0, 50, 50, 25, 25, 50, 50);

Функції
^^^^^^^

.. js:function:: geometry.intersect_lines(ax, ay, bx, by, cx, cy, dx, dy)

    Перевіряє, чи перетинаються відрізки AB та CD.

    :returns: ``true``, якщо відрізки перетинаються.
    :rtype: boolean

.. js:function:: geometry.intersect_aabb(ax, ay, aw, ah, bx, by, bw, bh)

    Перевіряє, чи перетинаються два прямокутники (AABB).

    :param number ax: X першого прямокутника.
    :param number ay: Y першого прямокутника.
    :param number aw: Ширина першого прямокутника.
    :param number ah: Висота першого прямокутника.
    :param number bx: X другого прямокутника.
    :param number by: Y другого прямокутника.
    :param number bw: Ширина другого прямокутника.
    :param number bh: Висота другого прямокутника.
    :returns: ``true``, якщо прямокутники перетинаються.
    :rtype: boolean
