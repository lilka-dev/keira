---@meta

---Апаратний ШІМ (PWM) на основі периферії ESP32 LEDC.
---
---Зверніть увагу: системний бузер також використовує LEDC, тому радимо
---використовувати вищі номери каналів (наприклад, ``2``..``15``), щоб уникнути конфлікту.
---@class pwm
pwm = {}

---Налаштовує канал ШІМ.
---@param channel integer номер каналу (``0``..``15``)
---@param freq integer частота в Гц
---@param resolution_bits integer? розрядність (``1``..``16``, за замовчуванням ``8``)
---@return integer actual фактична встановлена частота в Гц
function pwm.setup(channel, freq, resolution_bits) end

---Прив'язує GPIO-пін до каналу ШІМ.
---@param pin integer номер піна
---@param channel integer номер каналу
function pwm.attach_pin(pin, channel) end

---Відв'язує GPIO-пін від каналу ШІМ.
---@param pin integer номер піна
function pwm.detach_pin(pin) end

---Встановлює коефіцієнт заповнення (duty) для каналу.
---@param channel integer номер каналу
---@param duty integer значення заповнення (``0``..``2^resolution - 1``)
function pwm.write(channel, duty) end

---Генерує тон заданої частоти на каналі.
---@param channel integer номер каналу
---@param freq integer частота в Гц
---@return integer actual фактична встановлена частота в Гц
function pwm.write_tone(channel, freq) end

---Читає поточне значення заповнення (duty) каналу.
---@param channel integer номер каналу
---@return integer
function pwm.read(channel) end

return pwm
