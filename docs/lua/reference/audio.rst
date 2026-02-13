``audio`` - Аудіо
-----------------

Функції для відтворення аудіо-файлів через звуковий модуль (I2S).

Підтримувані формати: MOD, WAV, MP3, AAC, FLAC.

.. note:: Відтворення відбувається у фоновому режимі і не блокує виконання програми.

Приклад:

.. code-block:: lua
    :linenos:

    local music = resources.load_audio("song.mod")
    audio.play(music)
    util.sleep(5000)
    audio.stop()
    resources.delete(music)

.. lua:autoclass:: audio
