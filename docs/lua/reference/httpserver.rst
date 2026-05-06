``httpserver`` - HTTP-сервер
----------------------------

Функції для запуску вбудованого HTTP-сервера на Лілці.

Модуль ``httpserver`` дозволяє приймати HTTP-запити від інших пристроїв у мережі, обробляти їх та надсилати відповіді.
Для роботи сервера Лілка повинна бути підключена до WiFi (див. :doc:`wifi`).

Приклад простого HTTP-сервера, який відповідає на запити:

.. code-block:: lua
    :linenos:

    wifi.connect("MyWifi", "password")

    local server_fd = httpserver.listen(80)

    function lilka.update(delta)
        local req, err = httpserver.accept(server_fd, 0)
        if req == nil then
            return
        end

        if req.path == "/" then
            httpserver.respond(req.fd, 200, {["Content-Type"] = "text/plain"}, "Привіт від Лілки!")
        else
            httpserver.respond(req.fd, 404, {}, "Not Found")
        end
    end

    function lilka.draw()
        display.fill_screen(display.color565(0, 0, 0))
        display.set_cursor(0, 16)
        display.print("HTTP-сервер запущено")
    end

.. lua:autoclass:: httpserver
.. lua:autoclass:: HttpRequest
