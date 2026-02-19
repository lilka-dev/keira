---@meta

---@class audio
audio = {}

---Починає відтворення аудіо-файлу.
---
---Підтримувані формати: MOD, WAV, MP3, AAC, FLAC.
---
---Якщо вже щось відтворюється, поточне аудіо буде зупинено.
---
---@param sound table аудіо-ресурс, завантажений через ``resources.load_audio()``
---@usage
--- local music = resources.load_audio("song.mod")
--- audio.play(music)
--- -- Підтримуються різні формати:
--- local sfx = resources.load_audio("sound.wav")
--- local track = resources.load_audio("track.mp3")
function audio.play(sound) end

---Зупиняє відтворення аудіо.
function audio.stop() end

---Ставить відтворення на паузу.
function audio.pause() end

---Відновлює відтворення після паузи.
function audio.resume() end

---Встановлює гучність відтворення.
---
---@param volume number гучність від 0.0 до 4.0 (за замовчуванням — системна гучність)
function audio.set_volume(volume) end

---Повертає поточну гучність відтворення.
---
---@return number
function audio.get_volume() end

---Повертає ``true``, якщо аудіо зараз відтворюється.
---
---@return boolean
function audio.is_playing() end

return audio
