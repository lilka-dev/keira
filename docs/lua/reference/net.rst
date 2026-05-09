``net`` - TCP-сокети
---------------------

Функції для роботи з TCP-з'єднаннями на низькому рівні.

Модуль ``net`` надає доступ до сирих TCP-сокетів — як у ролі клієнта (підключення до сервера), так і у ролі сервера (очікування вхідних підключень).
Для роботи з мережею Лілка повинна бути підключена до WiFi (див. :doc:`wifi`).

.. note::
    Для роботи з HTTP-запитами використовуйте модуль :doc:`http`.
    Для побудови HTTP-сервера зручніше використовувати :doc:`httpserver`.
    Модуль ``net`` корисний, коли потрібно реалізувати власний протокол поверх TCP.

Приклад клієнтського підключення:

.. code-block:: lua
    :linenos:

    wifi.connect("MyWifi", "password")

    local fd, err = net.connect("example.com", 80)
    if fd == nil then
        error("Помилка підключення: " .. err)
    end

    net.send(fd, "GET / HTTP/1.0\r\nHost: example.com\r\n\r\n")

    local data, err = net.receive(fd, 1024)
    if data then
        print(data)
    end

    net.close(fd)

Приклад TCP-сервера:

.. code-block:: lua
    :linenos:

    wifi.connect("MyWifi", "password")

    local server_fd = net.listen(1234)

    function lilka.update(delta)
        local client_fd, client_ip = net.accept(server_fd, 0)
        if client_fd == nil then
            return
        end

        print("Підключився:", client_ip)
        local data = net.receive(client_fd, 256)
        if data then
            net.send(client_fd, "Отримано: " .. data)
        end
        net.close(client_fd)
    end

    function lilka.draw()
        display.fill_screen(display.color565(0, 0, 0))
        display.set_cursor(0, 16)
        display.print("TCP-сервер запущено")
    end

.. lua:autoclass:: net
