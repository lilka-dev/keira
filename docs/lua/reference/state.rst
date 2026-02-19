``state`` - Збереження стану програми
-------------------------------------

Таблиця ``state`` автоматично зберігається при завершенні програми.

Методи ``save()``, ``reset()``, ``clear()`` та властивість ``path`` є вбудованими і захищеними від перезапису.

Приклад:

.. code-block:: lua
    :linenos:

    state = state or { score = 0 }
    state.score = state.score + 1
    state.save() -- зберегти зараз, не чекаючи завершення
    state.reset() -- повернути останній збережений стан
    state.clear() -- видалити файл стану, state стає nil
    console.print(state.path) -- шлях до файлу стану

.. lua:autoclass:: state
