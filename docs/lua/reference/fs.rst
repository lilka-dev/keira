``fs`` - Робота з файловою системою
-------------------------------

Функції для роботи читання/запису файлів.

Приклад:

.. code-block:: lua
    :linenos:

    local file = fs.open("file.txt", "w")
    file:write("Hello, world!")

.. lua:autoclass:: fs

.. lua:autoclass:: File
