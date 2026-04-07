``controller`` — Контролер
--------------------------

Функції для зчитування стану кнопок.

Приклад:

.. code-block:: javascript
    :linenos:

    let state = controller.get_state();
    if (state.a.just_pressed) {
        console.print("Button A pressed!");
    }

Функції
^^^^^^^

.. js:function:: controller.get_state()

    Повертає поточний стан усіх кнопок.

    :returns: Об'єкт з полями: ``up``, ``down``, ``left``, ``right``, ``a``, ``b``, ``c``, ``d``, ``select``, ``start``.
              Кожне поле — об'єкт з властивостями:

              - ``pressed`` (``boolean``) — чи натиснута кнопка зараз.
              - ``just_pressed`` (``boolean``) — чи щойно натиснута.
              - ``just_released`` (``boolean``) — чи щойно відпущена.
    :rtype: object
