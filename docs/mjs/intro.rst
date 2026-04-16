.. _mjs-intro:

Написання програм на mJS (JavaScript)
======================================

Keira має вбудовану віртуальну машину **mJS** — легковісний рушій JavaScript, розроблений для мікроконтролерів. Це дозволяє писати та виконувати програми на JavaScript прямо з SD-картки, без необхідності компіляції чи перепрошивання Лілки.

.. warning:: mJS підтримує підмножину ECMAScript (ES6). Деякі можливості повноцінного JavaScript (наприклад, ``class``, ``async/await``, ``import``) **не підтримуються**. Детальніше: https://github.com/cesanta/mjs

.. contents::
    :local:
    :depth: 1

.. _mjs-example:

Приклад програми
----------------

Модулі завантажуються автоматично — не потрібно писати жодних ``require(...)`` чи ``import``. Ось приклад простої програми на mJS:

.. code-block:: javascript
    :linenos:

    // Заповнюємо екран чорним кольором
    display.fill_screen(display.color565(0, 0, 0));

    // Виводимо текст "Hello, world!" на екран
    display.set_cursor(0, 32);
    display.print("Hello, world!");
    display.queue_draw();

    // Чекаємо та завершуємо
    util.sleep(3);

Ви можете зберегти цей код у файл з розширенням ``.js`` на SD-картці, а потім виконати його, обравши його в браузері SD-картки.

Повний перелік доступних модулів та їх функцій можна знайти в розділі :doc:`reference/index`.

.. _mjs-differences:

Відмінності від Lua API
------------------------

Модулі mJS мають такий самий набір функцій та назви, як і :ref:`Lua API <lua-intro>`, з деякими відмінностями:

- **Синтаксис**: JavaScript замість Lua (``let`` замість ``local``, ``//`` замість ``--``, фігурні дужки замість ``end`` тощо).
- **Масиви**: Починаються з індексу **0** (а не з 1, як у Lua).
- **Немає автоматичних колбеків**: На відміну від Lua, де існують ``lilka.init()``, ``lilka.update()`` та ``lilka.draw()``, у mJS скрипт виконується послідовно від початку до кінця. Для ігрового циклу використовуйте ``while`` або ``for`` з ручними викликами ``display.queue_draw()`` та ``controller.get_state()``.
- **Модуль console**: Замість ``print()`` використовуйте ``console.print()``.

.. _mjs-gameloop:

Приклад ігрового циклу
----------------------

.. code-block:: javascript
    :linenos:

    let running = true;

    while (running) {
        // Зчитуємо стан кнопок
        let state = controller.get_state();
        if (state.a.just_pressed) {
            running = false;
        }

        // Малюємо
        display.fill_screen(colors.black);
        display.set_cursor(0, 32);
        display.print("Press A to exit");
        display.queue_draw();

        // Невелика затримка
        util.sleep(0.016);
    }
