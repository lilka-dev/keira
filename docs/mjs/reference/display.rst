``display`` — Дисплей
---------------------

Основні функції для роботи з дисплеєм.

Приклад:

.. code-block:: javascript
    :linenos:

    display.set_cursor(0, 0);
    display.print("Hello, world!");

    let color = display.color565(255, 0, 0);
    display.draw_line(0, 0, 100, 100, color);

    let face = resources.load_image("face.bmp", display.color565(0, 0, 0));
    display.draw_image(face, 50, 80);

    display.fill_rect(0, 0, 100, 100, color);
    display.queue_draw();

Константи
^^^^^^^^^

.. js:attribute:: display.width

    Ширина полотна у пікселях (``number``).

.. js:attribute:: display.height

    Висота полотна у пікселях (``number``).

Об'єкт ``colors`` містить стандартні кольори у форматі RGB565:

- ``colors.black``, ``colors.white``, ``colors.red``, ``colors.green``, ``colors.blue``
- ``colors.cyan``, ``colors.magenta``, ``colors.yellow``
- ``colors.midnight_blue``, ``colors.orange_red``

Функції
^^^^^^^

.. js:function:: display.color565(r, g, b)

    Конвертує колір з формату RGB888 (0–255) у RGB565.

    :param number r: Червоний компонент (0–255).
    :param number g: Зелений компонент (0–255).
    :param number b: Синій компонент (0–255).
    :returns: Колір у форматі RGB565.
    :rtype: number

.. js:function:: display.set_cursor(x, y)

    Встановлює позицію текстового курсора.

    :param number x: Координата X.
    :param number y: Координата Y.

.. js:function:: display.set_font(name)

    Встановлює шрифт. Доступні шрифти: ``"4x6"``, ``"5x7"``, ``"5x8"``, ``"6x12"``, ``"6x13"``, ``"7x13"``, ``"8x13"``, ``"9x15"``, ``"10x20"``.

    :param string name: Назва шрифту.

.. js:function:: display.set_text_size(size)

    Встановлює множник розміру тексту.

    :param number size: Множник розміру.

.. js:function:: display.set_text_color(fg[, bg])

    Встановлює колір тексту.

    :param number fg: Колір переднього плану (RGB565).
    :param number bg: *(необов'язково)* Колір фону (RGB565).

.. js:function:: display.set_text_bound(x, y, w, h)

    Встановлює обмежувальний прямокутник для тексту.

    :param number x: Координата X.
    :param number y: Координата Y.
    :param number w: Ширина.
    :param number h: Висота.

.. js:function:: display.print(...)

    Виводить текст на полотні в позиції курсора. Приймає довільну кількість аргументів (рядки або числа).

.. js:function:: display.fill_screen(color)

    Заповнює весь екран заданим кольором.

    :param number color: Колір (RGB565).

.. js:function:: display.draw_pixel(x, y, color)

    Малює один піксель.

    :param number x: Координата X.
    :param number y: Координата Y.
    :param number color: Колір (RGB565).

.. js:function:: display.draw_line(x0, y0, x1, y1, color)

    Малює лінію між двома точками.

    :param number x0: X початку.
    :param number y0: Y початку.
    :param number x1: X кінця.
    :param number y1: Y кінця.
    :param number color: Колір (RGB565).

.. js:function:: display.draw_rect(x, y, w, h, color)

    Малює контур прямокутника.

.. js:function:: display.fill_rect(x, y, w, h, color)

    Малює заповнений прямокутник.

.. js:function:: display.draw_circle(x, y, r, color)

    Малює контур кола.

    :param number x: Координата X центру.
    :param number y: Координата Y центру.
    :param number r: Радіус.
    :param number color: Колір (RGB565).

.. js:function:: display.fill_circle(x, y, r, color)

    Малює заповнене коло.

.. js:function:: display.draw_triangle(x0, y0, x1, y1, x2, y2, color)

    Малює контур трикутника.

.. js:function:: display.fill_triangle(x0, y0, x1, y1, x2, y2, color)

    Малює заповнений трикутник.

.. js:function:: display.draw_ellipse(x, y, rx, ry, color)

    Малює контур еліпса.

    :param number x: Координата X центру.
    :param number y: Координата Y центру.
    :param number rx: Горизонтальний радіус.
    :param number ry: Вертикальний радіус.
    :param number color: Колір (RGB565).

.. js:function:: display.fill_ellipse(x, y, rx, ry, color)

    Малює заповнений еліпс.

.. js:function:: display.draw_arc(x, y, r1, r2, startAngle, endAngle, color)

    Малює контур дуги.

    :param number x: Координата X центру.
    :param number y: Координата Y центру.
    :param number r1: Внутрішній радіус.
    :param number r2: Зовнішній радіус.
    :param number startAngle: Початковий кут.
    :param number endAngle: Кінцевий кут.
    :param number color: Колір (RGB565).

.. js:function:: display.fill_arc(x, y, r1, r2, startAngle, endAngle, color)

    Малює заповнену дугу.

.. js:function:: display.draw_image(image, x, y)

    Малює зображення в заданій позиції.

    :param object image: Об'єкт зображення (з ``resources.load_image()``).
    :param number x: Координата X.
    :param number y: Координата Y.

.. js:function:: display.draw_image_transformed(image, x, y, transform)

    Малює зображення з 2D-трансформацією.

    :param object image: Об'єкт зображення.
    :param number x: Координата X.
    :param number y: Координата Y.
    :param object transform: Об'єкт трансформації (з ``transforms``).

.. js:function:: display.queue_draw()

    Надсилає вміст полотна на фізичний дисплей. Викликайте цю функцію після завершення малювання кожного кадру.
