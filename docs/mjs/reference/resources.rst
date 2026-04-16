``resources`` — Ресурси
-----------------------

Функції для роботи з ресурсами (зображеннями, звуками, файлами).

Шляхи до файлів є відносними до директорії, в якій знаходиться скрипт.

Приклад:

.. code-block:: javascript
    :linenos:

    let face = resources.load_image("face.bmp", display.color565(0, 0, 0));
    display.draw_image(face, 50, 80);

    let music = resources.load_audio("song.mod");
    audio.play(music);

    // Після використання — звільніть пам'ять
    resources.delete(face);
    resources.delete(music);

Функції
^^^^^^^

.. js:function:: resources.load_image(path[, transparencyColor[, pivotX, pivotY]])

    Завантажує BMP-зображення з SD-картки.

    :param string path: Шлях до файлу (відносний до директорії скрипта).
    :param number transparencyColor: *(необов'язково)* Колір прозорості (RGB565). За замовчуванням: немає.
    :param number pivotX: *(необов'язково)* X координата точки обертання.
    :param number pivotY: *(необов'язково)* Y координата точки обертання.
    :returns: Об'єкт ``{width, height, pointer}`` або ``undefined`` при помилці.
    :rtype: object

.. js:function:: resources.rotate_image(image, angle, blankColor)

    Повертає повернуту копію зображення.

    :param object image: Об'єкт зображення.
    :param number angle: Кут обертання.
    :param number blankColor: Колір для незаповнених пікселів.
    :returns: Нове зображення ``{width, height, pointer}``.
    :rtype: object

.. js:function:: resources.flip_image_x(image)

    Повертає горизонтально перевернуту копію зображення.

    :param object image: Об'єкт зображення.
    :returns: Нове зображення ``{width, height, pointer}``.
    :rtype: object

.. js:function:: resources.flip_image_y(image)

    Повертає вертикально перевернуту копію зображення.

    :param object image: Об'єкт зображення.
    :returns: Нове зображення ``{width, height, pointer}``.
    :rtype: object

.. js:function:: resources.load_audio(path)

    Завантажує аудіо-файл з SD-картки.

    Підтримувані формати: ``.mod``, ``.wav``, ``.mp3``, ``.aac``, ``.flac``.

    :param string path: Шлях до файлу (відносний до директорії скрипта).
    :returns: Об'єкт ``{size, type, pointer}`` або ``undefined`` при помилці.
    :rtype: object

.. js:function:: resources.delete(resource)

    Звільняє пам'ять, виділену для ресурсу (зображення або звук). Якщо звук зараз відтворюється — зупиняє його.

    :param object resource: Об'єкт ресурсу для видалення.

.. js:function:: resources.read_file(path)

    Зчитує текстовий файл з SD-картки.

    :param string path: Шлях до файлу (відносний до директорії скрипта).
    :returns: Вміст файлу як рядок або ``undefined`` при помилці.
    :rtype: string

.. js:function:: resources.write_file(path, content)

    Записує текст у файл на SD-картці.

    :param string path: Шлях до файлу (відносний до директорії скрипта).
    :param string content: Текст для запису.
