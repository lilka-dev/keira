---@meta

---@class i2c
i2c = {}

---Ініціалізує шину I2C. Якщо піни не вказані, використовуються типові піни плати.
---@param sda integer? номер піна SDA
---@param scl integer? номер піна SCL
---@param freq integer? частота шини в Гц (за замовчуванням ``100000``)
function i2c.begin(sda, scl, freq) end

---Встановлює тактову частоту шини I2C.
---@param freq integer частота шини в Гц
function i2c.set_clock(freq) end

---Сканує шину та повертає таблицю знайдених 7-бітних адрес пристроїв.
---@return integer[]
function i2c.scan() end

---Записує дані пристроєві за адресою ``addr``.
---@param addr integer 7-бітна адреса пристрою
---@param data integer|integer[] один байт або таблиця байтів
---@return integer статус (``0`` - успіх)
function i2c.write(addr, data) end

---Читає ``count`` байтів з пристрою за адресою ``addr``.
---@param addr integer 7-бітна адреса пристрою
---@param count integer кількість байтів для читання
---@return integer[]
function i2c.read(addr, count) end

---Записує дані, а потім виконує читання ``count`` байтів з повторним стартом (repeated start).
---@param addr integer 7-бітна адреса пристрою
---@param wdata integer|integer[] один байт або таблиця байтів для запису
---@param count integer кількість байтів для читання
---@return integer[]
function i2c.write_read(addr, wdata, count) end

return i2c
