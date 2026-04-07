#include "mjswifi.h"
#include <WiFi.h>
#include <lilka.h>
#include "mjs.h"

// wifi.connect(ssid, password)
static void mjs_wifi_connect(struct mjs* mjs) {
    mjs_val_t arg0 = mjs_arg(mjs, 0);
    mjs_val_t arg1 = mjs_arg(mjs, 1);
    const char* ssid = mjs_get_cstring(mjs, &arg0);
    const char* password = mjs_get_cstring(mjs, &arg1);
    WiFi.begin(ssid, password);
    mjs_return(mjs, mjs_mk_undefined());
}

// wifi.get_status() -> number
static void mjs_wifi_get_status(struct mjs* mjs) {
    mjs_return(mjs, mjs_mk_number(mjs, WiFi.status()));
}

// wifi.disconnect()
static void mjs_wifi_disconnect(struct mjs* mjs) {
    WiFi.disconnect();
    mjs_return(mjs, mjs_mk_undefined());
}

// wifi.scan() -> array of SSIDs
static void mjs_wifi_scan(struct mjs* mjs) {
    int n = WiFi.scanNetworks();
    mjs_val_t arr = mjs_mk_array(mjs);
    for (int i = 0; i < n; i++) {
        mjs_array_push(mjs, arr, mjs_mk_string(mjs, WiFi.SSID(i).c_str(), ~0, 1));
    }
    mjs_return(mjs, arr);
}

// wifi.get_rssi(index) -> number
static void mjs_wifi_get_rssi(struct mjs* mjs) {
    int idx = mjs_get_int(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, WiFi.RSSI(idx)));
}

// wifi.get_encryption_type(index) -> number
static void mjs_wifi_get_encryption_type(struct mjs* mjs) {
    int idx = mjs_get_int(mjs, mjs_arg(mjs, 0));
    mjs_return(mjs, mjs_mk_number(mjs, WiFi.encryptionType(idx)));
}

// wifi.get_mac() -> string
static void mjs_wifi_get_mac(struct mjs* mjs) {
    byte mac[6];
    WiFi.macAddress(mac);
    String s;
    for (byte i = 0; i < sizeof(mac); i++) {
        char buf[3];
        sprintf(buf, "%02X", mac[i]);
        s += buf;
        if (i < 5) s += ':';
    }
    mjs_return(mjs, mjs_mk_string(mjs, s.c_str(), ~0, 1));
}

// wifi.get_local_ip() -> string
static void mjs_wifi_get_local_ip(struct mjs* mjs) {
    IPAddress ip = WiFi.localIP();
    mjs_return(mjs, mjs_mk_string(mjs, ip.toString().c_str(), ~0, 1));
}

// wifi.set_config(ip, gateway, subnet, dns1, dns2)
static void mjs_wifi_set_config(struct mjs* mjs) {
    mjs_val_t a0 = mjs_arg(mjs, 0);
    mjs_val_t a1 = mjs_arg(mjs, 1);
    mjs_val_t a2 = mjs_arg(mjs, 2);
    mjs_val_t a3 = mjs_arg(mjs, 3);
    mjs_val_t a4 = mjs_arg(mjs, 4);
    IPAddress ip, gateway, subnet, dns1, dns2;
    ip.fromString(mjs_get_cstring(mjs, &a0));
    gateway.fromString(mjs_get_cstring(mjs, &a1));
    subnet.fromString(mjs_get_cstring(mjs, &a2));
    dns1.fromString(mjs_get_cstring(mjs, &a3));
    dns2.fromString(mjs_get_cstring(mjs, &a4));
    WiFi.config(ip, gateway, subnet, dns1, dns2);
    mjs_return(mjs, mjs_mk_undefined());
}

void mjs_wifi_register(struct mjs* mjs) {
    mjs_val_t wifi = mjs_mk_object(mjs);
    mjs_set(mjs, wifi, "connect", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_wifi_connect));
    mjs_set(mjs, wifi, "get_status", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_wifi_get_status));
    mjs_set(mjs, wifi, "disconnect", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_wifi_disconnect));
    mjs_set(mjs, wifi, "scan", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_wifi_scan));
    mjs_set(mjs, wifi, "get_rssi", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_wifi_get_rssi));
    mjs_set(
        mjs, wifi, "get_encryption_type", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_wifi_get_encryption_type)
    );
    mjs_set(mjs, wifi, "get_mac", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_wifi_get_mac));
    mjs_set(mjs, wifi, "get_local_ip", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_wifi_get_local_ip));
    mjs_set(mjs, wifi, "set_config", ~0, mjs_mk_foreign_func(mjs, (mjs_func_ptr_t)mjs_wifi_set_config));
    mjs_val_t global = mjs_get_global(mjs);
    mjs_set(mjs, global, "wifi", ~0, wifi);
}
