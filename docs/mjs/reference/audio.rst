``audio`` — Аудіо
-----------------

Функції для відтворення аудіо-файлів через звуковий модуль (I2S).

Підтримувані формати: MOD, WAV, MP3, AAC, FLAC.

.. note:: Відтворення відбувається у фоновому режимі і не блокує виконання програми.

Приклад:

.. code-block:: javascript
    :linenos:

    let music = resources.load_audio("song.mod");
    audio.play(music);
    util.sleep(5);
    audio.stop();
    resources.delete(music);

Функції
^^^^^^^

.. js:function:: audio.play(sound)

    Відтворює аудіо-ресурс.

    :param object sound: Об'єкт звуку (з ``resources.load_audio()``).

.. js:function:: audio.stop()

    Зупиняє відтворення.

.. js:function:: audio.pause()

    Ставить на паузу.

.. js:function:: audio.resume()

    Продовжує відтворення після паузи.

.. js:function:: audio.set_volume(gain)

    Встановлює гучність.

    :param number gain: Гучність (дробове число).

.. js:function:: audio.get_volume()

    Повертає поточну гучність.

    :returns: Гучність.
    :rtype: number

.. js:function:: audio.is_playing()

    Перевіряє, чи відбувається зараз відтворення.

    :returns: ``true``, якщо аудіо відтворюється.
    :rtype: boolean
