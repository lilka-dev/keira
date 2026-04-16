``math`` — Арифметичні функції
------------------------------

Цей модуль містить функції для виконання різних арифметичних та тригонометричних операцій.

Константи
^^^^^^^^^

.. js:attribute:: math.pi

    Число π (~3.14159).

.. js:attribute:: math.e

    Число Ейлера (~2.71828).

.. js:attribute:: math.tau

    Число 2π (~6.28318).

Функції
^^^^^^^

.. js:function:: math.random([max_or_min[, max]])

    Генерує випадкове число.

    - Без аргументів: випадкове дробове число в діапазоні [0, 1).
    - З одним аргументом ``max``: випадкове ціле число в діапазоні [0, max).
    - З двома аргументами ``min``, ``max``: випадкове ціле число в діапазоні [min, max).

.. js:function:: math.clamp(value, min, max)

    Обмежує значення до діапазону [min, max].

.. js:function:: math.lerp(a, b, t)

    Лінійна інтерполяція: ``a + (b - a) * t``.

.. js:function:: math.map(value, in_min, in_max, out_min, out_max)

    Відображає значення з одного діапазону на інший.

.. js:function:: math.abs(value)

    Повертає абсолютне значення.

.. js:function:: math.sign(value)

    Повертає знак числа: ``1``, ``-1`` або ``0``.

.. js:function:: math.sqrt(value)

    Квадратний корінь.

.. js:function:: math.pow(base, exp)

    Піднесення до степеня.

.. js:function:: math.min(array)

    Повертає мінімальне значення з масиву.

.. js:function:: math.max(array)

    Повертає максимальне значення з масиву.

.. js:function:: math.sum(array)

    Повертає суму елементів масиву.

.. js:function:: math.avg(array)

    Повертає середнє значення елементів масиву.

.. js:function:: math.floor(value)

    Округлення вниз.

.. js:function:: math.ceil(value)

    Округлення вгору.

.. js:function:: math.round(value)

    Округлення до найближчого цілого.

.. js:function:: math.sin(x)

    Синус (аргумент у радіанах).

.. js:function:: math.cos(x)

    Косинус (аргумент у радіанах).

.. js:function:: math.tan(x)

    Тангенс (аргумент у радіанах).

.. js:function:: math.asin(x)

    Арксинус.

.. js:function:: math.acos(x)

    Арккосинус.

.. js:function:: math.atan(x)

    Арктангенс.

.. js:function:: math.atan2(y, x)

    Арктангенс двох аргументів.

.. js:function:: math.log(value[, base])

    Логарифм. Без другого аргументу — натуральний логарифм. З ``base`` — логарифм за заданою основою.

.. js:function:: math.deg(radians)

    Конвертує радіани у градуси.

.. js:function:: math.rad(degrees)

    Конвертує градуси у радіани.

.. js:function:: math.norm(x, y)

    Нормалізує 2D-вектор.

    :returns: Масив ``[nx, ny]``.
    :rtype: Array

.. js:function:: math.len(x, y)

    Довжина 2D-вектора.

.. js:function:: math.dist(x1, y1, x2, y2)

    Евклідова відстань між двома точками.

.. js:function:: math.rotate(x, y, angle)

    Обертає 2D-вектор на заданий кут (у градусах, за годинниковою стрілкою, Y вниз).

    :returns: Масив ``[rx, ry]``.
    :rtype: Array
