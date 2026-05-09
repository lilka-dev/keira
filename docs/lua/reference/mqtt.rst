``mqtt`` - Протокол MQTT
------------------------

Функції для роботи з протоколом `MQTT <https://uk.wikipedia.org/wiki/MQTT>`_ — легковажним протоколом обміну повідомленнями для IoT-пристроїв.

Для роботи з MQTT Лілка повинна бути підключена до WiFi (див. :doc:`wifi`).

Приклад підключення до MQTT-брокера, підписки на топік та публікації повідомлень:

.. code-block:: lua
    :linenos:

    wifi.connect("MyWifi", "password")

    local client, err = mqtt.connect({
        host = "broker.hivemq.com",
        port = 1883,
        client_id = "lilka-test",
    })
    if client == nil then
        error("Не вдалося підключитись: " .. err)
    end

    mqtt.subscribe(client, "lilka/in")

    function lilka.update(delta)
        local msg = mqtt.receive(client)
        if msg then
            print("Отримано:", msg.topic, msg.payload)
        end

        local state = controller.get_state()
        if state.a.just_pressed then
            mqtt.publish(client, "lilka/out", "Привіт!")
        end
        if state.b.just_pressed then
            util.exit()
        end
    end

    function lilka.draw()
        display.fill_screen(display.color565(0, 0, 0))
        display.set_cursor(0, 16)
        display.print("MQTT: підключено")
    end

.. lua:autoclass:: mqtt
.. lua:autoclass:: MqttClient
.. lua:autoclass:: MqttMessage
