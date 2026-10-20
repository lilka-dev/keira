---@meta

---Шина SPI для зовнішніх пристроїв (користувацька шина SPI2 / FSPI).
---
---Доступна лише на Лілці v2. Шина дисплея та SD-картки (SPI1) не використовується.
---@class spi
---@field MODE0 integer режим SPI 0
---@field MODE1 integer режим SPI 1
---@field MODE2 integer режим SPI 2
---@field MODE3 integer режим SPI 3
spi = {}

---Ініціалізує шину SPI. Якщо піни не вказані, використовуються типові піни шини.
---@param sck integer? номер піна SCK (тактовий сигнал)
---@param miso integer? номер піна MISO
---@param mosi integer? номер піна MOSI
function spi.begin(sck, miso, mosi) end

---Передає дані по шині SPI (повний дуплекс) і повертає прийняті дані.
---
---Лінію вибору пристрою (CS) потрібно керувати вручну через модуль ``gpio``.
---@param data integer|integer[] один байт або таблиця байтів
---@param frequency integer? частота в Гц (за замовчуванням ``4000000``)
---@param mode integer? режим SPI, наприклад ``spi.MODE0`` (за замовчуванням ``spi.MODE0``)
---@return integer|integer[] прийнятий байт або таблиця байтів
function spi.transfer(data, frequency, mode) end

---Завершує роботу з шиною SPI.
function spi.close() end

return spi
