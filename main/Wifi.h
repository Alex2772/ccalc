//
// Created by alex2772 on 12.12.17.
//

#pragma once

#include <string>
#include <functional>
#include <esp_wifi.h>
#include <vector>

namespace Wifi {
    enum State {
        CONNECTED_GOT_IP,
        CONNECTED,
        CONNECTING,
        IDLE,
        DISABLED,
    };

    void connect(const std::string& ssid, const std::string& pwd);
    State getStatus();
    const std::string getWifiSsid();
    struct ip_address {
        uint8_t b1, b2, b3, b4;
    };
    ip_address getIpAddress();
    void disconnect();
    void init();
    bool isConnected();

    void enable();
    void disable();
    bool isEnabled();

    void setIp(uint32_t ip);

    void setState(State state);

    bool isInetAvailable();

    void scan(void (*callback)(const std::vector<wifi_ap_record_t> &));
}