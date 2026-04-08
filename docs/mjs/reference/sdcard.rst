``sdcard`` — SD-картка
----------------------

Функції для роботи з файлами на SD-картці.

Приклад:

.. code-block:: javascript
    :linenos:

    // Список файлів у кореневій директорії
    let files = sdcard.ls("/");
    for (let i = 0; i < files.length; i++) {
        console.print(files[i]);
    }

    // Читання файлу
    let f = sdcard.open("/test.txt", "r");
    let content = f.read(f, 1024);
    console.print(content);
    f.close(f);

Функції
^^^^^^^

.. js:function:: sdcard.ls(path)

    Повертає масив імен файлів і папок у заданій директорії.

    :param string path: Шлях до директорії.
    :returns: Масив рядків або ``undefined``, якщо SD-картка недоступна.
    :rtype: Array

.. js:function:: sdcard.remove(path)

    Видаляє файл з SD-картки.

    :param string path: Шлях до файлу.

.. js:function:: sdcard.rename(oldName, newName)

    Перейменовує або переміщує файл.

    :param string oldName: Поточний шлях.
    :param string newName: Новий шлях.

.. js:function:: sdcard.open(path[, mode])

    Відкриває файл і повертає файловий об'єкт.

    :param string path: Шлях до файлу.
    :param string mode: *(необов'язково)* Режим відкриття (``"r"``, ``"w"``, ``"a"`` тощо). За замовчуванням: ``"r"``.
    :returns: Файловий об'єкт з методами (див. нижче).
    :rtype: object

Файловий об'єкт
^^^^^^^^^^^^^^^^

Методи файлового об'єкта, поверненого ``sdcard.open()``. У кожен метод першим аргументом передається сам файловий об'єкт.

.. js:function:: file.size(file)

    Повертає розмір файлу у байтах.

    :returns: Розмір.
    :rtype: number

.. js:function:: file.seek(file, offset)

    Переміщує покажчик у файлі на вказану позицію.

    :param number offset: Позиція у байтах від початку файлу.

.. js:function:: file.read(file, maxBytes)

    Зчитує до ``maxBytes`` байтів з файлу як рядок.

    :param number maxBytes: Максимальна кількість байтів.
    :returns: Зчитані дані.
    :rtype: string

.. js:function:: file.write(file, text)

    Записує текст у файл.

    :param string text: Текст для запису.

.. js:function:: file.exists(file)

    Перевіряє, чи файловий покажчик валідний.

    :returns: ``true``, якщо файл відкрито успішно.
    :rtype: boolean

.. js:function:: file.close(file)

    Закриває файл. Обов'язково викликайте після завершення роботи з файлом.
