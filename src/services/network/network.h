#pragma once

#include <WiFi.h>

#include "keira/service.h"
#include "keira/servicemanager.h"

#include "keira/mutex.h"

enum NetworkState {
    NETWORK_STATE_DISABLED,
    NETWORK_STATE_OFFLINE,
    NETWORK_STATE_CONNECTING,
    NETWORK_STATE_ONLINE,
};

class NetworkService : public Service {
public:
    ~NetworkService();

    bool connect(String ssid);
    void connect(String ssid, String password);
    // This one is special, cause takes stuff from NVS
    String getPassword(String ssid);
    KMTX_GETER(NetworkState, networkState, mtxNetwork);
    KMTX_GETER(int8_t, signalStrength, mtxNetwork);
    KMTX_GETER(String, ipAddr, mtxNetwork);

private:
    // util
    static String hash(String input);

    void run() override;
    void autoConnect();

    KMTX_SETER(NetworkState, networkState, mtxNetwork);
    KMTX_SETER(int, disconnectReason, mtxNetwork);
    KMTX_SETER(int8_t, signalStrength, mtxNetwork);
    KMTX_SETER(String, lastPassword, mtxNetwork);
    KMTX_SETER(String, ipAddr, mtxNetwork);

    SemaphoreHandle_t mtxNetwork = xSemaphoreCreateMutex();

    NetworkState networkState = NETWORK_STATE_OFFLINE;
    int disconnectReason = 0;
    int8_t signalStrength = 0; // Value in range [0,3]
    String lastPassword = ""; // wtf is that
    String ipAddr = "";
};
