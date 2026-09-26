---@meta

---@class service
---Фонові сервіси - скрипти, які працюють у фоні, поки ви користуєтесь іншими програмами.
---
---Фоновий сервіс не має доступу до екрана та кнопок, тому модулі ``display``, ``controller``, ``resources``, ``transforms``, ``UI`` та ``audio`` в ньому недоступні.
---Помилки сервісу виводяться в серійний порт.
---
---Сервіс спочатку виконує скрипт повністю, а потім викликає ``lilka.init()`` та раз на ``service.interval`` секунд - ``lilka.update(delta)``, якщо ці функції визначені.
---Можна також просто написати нескінченний цикл з ``util.sleep()``.
---
---Ім'я сервісу складається з префікса мови та імені файлу без розширення: ``lua:autorun``, ``js:blink``. Одночасно може працювати лише один сервіс з таким іменем.
---
---**Автозапуск:** якщо в корені SD-карти є файли ``autorun.lua`` або ``autorun.js``, вони запускаються як фонові сервіси при старті Keira. Автозапуск вимикається в меню "Налаштування" → "Сервіси" → "Автозапуск".
---
---.. note:: Сервіс зупиняється під час наступного ``util.sleep()`` або, для Lua, після кількох інструкцій. Блокуючі виклики (наприклад, ``http.execute``) не перериваються. JavaScript-сервіс зупиняється лише під час ``util.sleep()``.
---
---@usage
--- -- autorun.lua: блимає світлодіодом на піні 10 у фоні
--- local on = false
--- gpio.set_mode(10, gpio.OUTPUT)
---
--- service.interval = 0.5 -- викликати lilka.update() двічі на секунду
---
--- function lilka.update(delta)
---     on = not on
---     gpio.write(10, on and gpio.HIGH or gpio.LOW)
--- end
service = {}

---Ім'я поточного сервісу, наприклад ``"lua:autorun"``. Доступне лише всередині фонового сервісу.
---@type string
service.name = nil

---Інтервал між викликами ``lilka.update(delta)`` в секундах. За замовчуванням - 0.1. Доступний лише всередині фонового сервісу.
---@type number
service.interval = 0.1

---Запускає Lua (``.lua``) або JavaScript (``.js``) скрипт як фоновий сервіс.
---Відносний шлях рахується від директорії поточного скрипта.
---@param path string шлях до скрипта
---@return string|nil name ім'я запущеного сервісу, або ``nil`` у разі помилки
---@return string|nil err опис помилки
---@usage
--- local name, err = service.start("blink.lua")
--- if name == nil then
---     print("Не вдалося запустити сервіс: " .. err)
--- end
function service.start(path) end

---Зупиняє фоновий сервіс з вказаним ім'ям. Щоб зупинити поточний сервіс, використовуйте ``util.exit()``.
---@param name string ім'я сервісу
---@return boolean stopped ``true``, якщо сервіс знайдено та попрошено зупинитись
---@usage
--- service.stop("lua:blink")
function service.stop(name) end

---Повертає імена всіх запущених фонових сервісів.
---@return string[]
---@usage
--- for _, name in ipairs(service.list()) do
---     print(name)
--- end
function service.list() end

---Перевіряє, чи працює фоновий сервіс з вказаним ім'ям.
---@param name string ім'я сервісу
---@return boolean
function service.is_running(name) end

return service
