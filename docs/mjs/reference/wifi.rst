``wifi`` — Робота з WiFi-мережами
---------------------------------

Функції для роботи з WiFi-мережами.

Приклад:

.. code-block:: javascript
    :linenos:

    let networks = wifi.scan();
    for (let i = 0; i < networks.length; i++) {
        console.print(networks[i]);
        console.print("RSSI:", wifi.get_rssi(i));
        console.print("Encryption:", wifi.get_encryption_type(i));
    }

Функції
^^^^^^^

.. js:function:: wifi.connect(ssid, password)

    Підключається до WiFi-мережі.

    :param string ssid: Назва мережі.
    :param string password: Пароль.

.. js:function:: wifi.get_status()

    Повертає код стану WiFi-з'єднання.

    :returns: Код стану (``WiFi.status()``).
    :rtype: number

.. js:function:: wifi.disconnect()

    Відключається від WiFi-мережі.

.. js:function:: wifi.scan()

    Сканує доступні WiFi-мережі.

    :returns: Масив рядків (SSID).
    :rtype: Array

.. js:function:: wifi.get_rssi(index)

    Повертає силу сигналу мережі за індексом з результатів сканування.

    :param number index: Індекс мережі.
    :returns: RSSI у дБм.
    :rtype: number

.. js:function:: wifi.get_encryption_type(index)

    Повертає тип шифрування мережі за індексом.

    :param number index: Індекс мережі.
    :returns: Код типу шифрування.
    :rtype: number

.. js:function:: wifi.get_mac()

    Повертає MAC-адресу пристрою.

    :returns: MAC-адреса у форматі ``"XX:XX:XX:XX:XX:XX"``.
    :rtype: string

.. js:function:: wifi.get_local_ip()

    Повертає локальну IP-адресу.

    :returns: IP-адреса.
    :rtype: string

.. js:function:: wifi.set_config(ip, gateway, subnet, dns1, dns2)

    Встановлює статичну конфігурацію мережі.

    :param string ip: IP-адреса.
    :param string gateway: Шлюз.
    :param string subnet: Маска підмережі.
    :param string dns1: Первинний DNS.
    :param string dns2: Вторинний DNS.
