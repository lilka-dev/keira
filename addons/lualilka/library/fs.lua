---@meta

---@class fs
fs = {}

---Повернути таблицю зі списком файлів та директорій за вказаним шляхом.
---
---@param path string шлях до директорії
---@return table
---@usage
--- local entries = fs.ls("/sd/folder")
--- for i = 0, #entries do
---     print(entries[i])
--- end
function fs.ls(path) end

---Створити директорію за вказаним шляхом
---
---@param path string шлях до файлу або директорії
---@usage
--- fs.mkpath("/sd/path/to/a/folder")
function fs.mkpath(path) end

---Видалити файл або директорію за вказаним шляхом.
---
---@param path string шлях до файлу або директорії
---@usage
--- fs.remove("/sd/file.txt")
function fs.remove(path) end

---Перейменувати файл або директорію.
---
---@param old_path string старий шлях до файлу або директорії
---@param new_path string новий шлях до файлу або директорії
---
---@usage
--- fs.rename("/sd/file.txt", "/sd/file2.txt")
function fs.rename(old_path, new_path) end

---Об'єднати частини шляху
---
---@param lpath string ліва половина шляху
---@param rpath string права половина шляху
---
---@usage
--- fs.joinpath("/sd", "path/on/sdcard/file.txt")
function fs.joinpath(lpath, rpath) end

---@class File
File = {}

---Відкрити файл за вказаним шляхом.
---
---@param path string шлях до файлу
---@param mode string режим відкриття файлу (за замовчуванням r) (див. функцію ``fopen`` у документації ANSI C)
---@return File
---
---@usage
--- local file = fs.open("/tmp/file.txt", "a+") -- Відкриває файл для додавання тексту
--- file:write("Hello, world!\n") -- Дописує текст в кінець файлу
function fs.open(path, mode) end

---Повернути розмір файлу.
---
---@return integer
function File:size() end

---Повернути наявність файлу.
---
---@return boolean
function File:exists() end

---Перемістити вказівник файлу на певну позицію.
---
---@param pos integer позиція в файлі
function File:seek(pos) end

---Прочитати з файлу.
---
---@param count integer максимальна кількість байт, які потрібно прочитати
---@return string
function File:read(count) end

---Записати у файл.
---
---@param content string дані, які потрібно записати
---@usage
--- local file = fs.open("/sd/file.txt", "w") -- Відкриває файл для запису
--- file:write("Hello, world!\n") -- Записує текст у файл
--- file:write("Привіт, світ!\n") -- Дописує текст у файл
function File:write(content) end

return fs
