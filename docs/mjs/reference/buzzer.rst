``buzzer`` — П'єзо-динамік
--------------------------

Функції для роботи з п'єзо-динаміком.

.. note:: Ці функції не блокують виконання програми: всі звуки та мелодії відтворюються в фоновому режимі.

Приклад:

.. code-block:: javascript
    :linenos:

    // Відтворити ноту C4 протягом 500 мс
    buzzer.play(notes.C4, 500);
    util.sleep(0.6);

    // Відтворити мелодію
    buzzer.play_melody(
        [[notes.C4, 4], [notes.E4, 4], [notes.G4, 2]],
        120
    );

Константи
^^^^^^^^^

Глобальний об'єкт ``notes`` містить константи для всіх музичних нот:

- ``notes.B0``
- ``notes.C1`` ... ``notes.B1``
- ``notes.C2`` ... ``notes.B2``
- ``notes.C3`` ... ``notes.B3``
- ``notes.C4`` ... ``notes.B4``
- ``notes.C5`` ... ``notes.B5``
- ``notes.C6`` ... ``notes.B6``
- ``notes.C7`` ... ``notes.B7``
- ``notes.C8``, ``notes.CS8``, ``notes.D8``, ``notes.DS8``
- ``notes.REST`` — пауза

Дієзи позначаються суфіксом ``S`` (наприклад, ``notes.CS4`` = C#4, ``notes.DS5`` = D#5).

Функції
^^^^^^^

.. js:function:: buzzer.play(freq[, duration])

    Відтворює тон із заданою частотою.

    :param number freq: Частота у Гц (або константа з ``notes``).
    :param number duration: *(необов'язково)* Тривалість у мілісекундах. Без цього параметру — нескінченно.

.. js:function:: buzzer.play_melody(tones, tempo)

    Відтворює мелодію.

    :param Array tones: Масив пар ``[частота, розмір]``. Розмір визначає тривалість ноти відносно темпу.
    :param number tempo: Темп у BPM.

.. js:function:: buzzer.stop()

    Зупиняє відтворення звуку.
