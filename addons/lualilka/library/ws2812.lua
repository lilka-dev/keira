---@meta

---Керування адресованими світлодіодами WS2812 ("NeoPixel") через периферію RMT.
---@class ws2812
ws2812 = {}

---Виводить кольори на стрічку світлодіодів WS2812, під'єднану до піна ``pin``.
---
---Кожен елемент таблиці ``colors`` - це ціле число у форматі ``0xRRGGBB``.
---@param pin integer номер піна даних (DIN)
---@param colors integer[] таблиця кольорів у форматі ``0xRRGGBB``
function ws2812.write(pin, colors) end

return ws2812
