``transforms`` — 2D-трансформації
----------------------------------

Функції для роботи з 2D-трансформаціями (обертання, масштабування тощо).

Приклад:

.. code-block:: javascript
    :linenos:

    let t = transforms.new();
    t = transforms.rotate(t, 45);
    t = transforms.scale(t, 2, 2);
    display.draw_image_transformed(image, 100, 100, t);
    transforms.delete(t);

Функції
^^^^^^^

.. js:function:: transforms.new()

    Створює нову одиничну трансформацію.

    :returns: Об'єкт трансформації ``{pointer}``.
    :rtype: object

.. js:function:: transforms.rotate(transform, angle)

    Повертає нову трансформацію, повернуту на заданий кут.

    :param object transform: Вхідна трансформація.
    :param number angle: Кут обертання у градусах.
    :returns: Нова трансформація.
    :rtype: object

.. js:function:: transforms.scale(transform, sx, sy)

    Повертає нову трансформацію, масштабовану по осях X та Y.

    :param object transform: Вхідна трансформація.
    :param number sx: Масштаб по X.
    :param number sy: Масштаб по Y.
    :returns: Нова трансформація.
    :rtype: object

.. js:function:: transforms.multiply(transform, other)

    Повертає добуток двох трансформацій.

    :param object transform: Перша трансформація.
    :param object other: Друга трансформація.
    :returns: Нова трансформація (t1 × t2).
    :rtype: object

.. js:function:: transforms.inverse(transform)

    Повертає обернену трансформацію.

    :param object transform: Вхідна трансформація.
    :returns: Обернена трансформація.
    :rtype: object

.. js:function:: transforms.vtransform(transform, x, y)

    Застосовує трансформацію до точки (x, y).

    :param object transform: Трансформація.
    :param number x: Координата X.
    :param number y: Координата Y.
    :returns: Масив ``[x, y]`` з трансформованими координатами.
    :rtype: Array

.. js:function:: transforms.get(transform)

    Повертає матрицю трансформації 2×2.

    :param object transform: Трансформація.
    :returns: Масив ``[[a, b], [c, d]]``.
    :rtype: Array

.. js:function:: transforms.set(transform, matrix)

    Встановлює матрицю трансформації 2×2.

    :param object transform: Трансформація.
    :param Array matrix: Масив ``[[a, b], [c, d]]``.

.. js:function:: transforms.delete(transform)

    Звільняє пам'ять, виділену для трансформації.

    :param object transform: Трансформація для видалення.
