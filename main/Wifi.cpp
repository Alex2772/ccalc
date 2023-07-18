#include <cstdint>
#include <esp_wifi.h>
#include <cstring>
#include <esp_system.h>
#include <esp_wifi_types.h>
#include "Wifi.h"
#include "Cloud.h"
#include "Socket.h"
#include "CCOSCore.h"
#include <esp_log.h>

//#define USE_NVS

extern "C" {
#include <freertos/FreeRTOS.h>
#include <freertos/timers.h>
}

Wifi::State _state = Wifi::DISABLED;

std::string __ssid;
std::string __pwd;

bool __wifi_init = true;

void Wifi::connect(const std::string &ssid, const std::string &pwd) {
    _state = Wifi::CONNECTING;
    __ssid = ssid;
    __pwd = pwd;
    esp_wifi_stop();
    wifi_config_t c;
    memset(&c, 0, sizeof(c));
    strcpy(reinterpret_cast<char *>(c.sta.ssid), ssid.c_str());
    strcpy(reinterpret_cast<char *>(c.sta.password), pwd.c_str());
    c.sta.scan_method = WIFI_ALL_CHANNEL_SCAN;
    c.sta.sort_method = WIFI_CONNECT_AP_BY_SIGNAL;
    esp_wifi_set_config(WIFI_IF_STA, &c);
    esp_wifi_start();
    esp_wifi_connect();

}
Wifi::State Wifi::getStatus() {
    return _state;
}

const std::string Wifi::getWifiSsid() {
    return __ssid;
}

Wifi::ip_address _ip;
Wifi::ip_address Wifi::getIpAddress() {
    return _ip;
}

void ::Wifi::disconnect() {
    __ssid.clear();
    __pwd.clear();

#ifdef USE_NVS
    if (!__wifi_init)
        CCOSCore::runOnUiThread([&]() {
            NVS::Accessor nvs = CCOSCore::getNVSHandle().open();

            nvs.write("wifi_ssid", __ssid);
            nvs.write("wifi_pwd", __pwd);
        });
#endif
    esp_wifi_disconnect();
    esp_wifi_stop();
}
bool _inet = false;
void wifi_timer(TimerHandle_t t) {
    Socket s("77.239.254.135", 4322);
    _inet = s.state == Socket::STATE_OK;
    s.close();
}
TimerHandle_t _checker;
void ::Wifi::init() {
    _checker = xTimerCreate("WifiCheckerTimer", 10000 / portTICK_PERIOD_MS, pdTRUE, 0, wifi_timer);

#ifdef USE_NVS
    CCOSCore::runOnUiThread([&]() {
        bool b = false;
        NVS::Accessor nvs = CCOSCore::getNVSHandle().open();
        nvs.read("wifi_enabled", b);
        if (b) {
            enable();
            std::string ssid, pwd;
            if (nvs.read("wifi_ssid", ssid) && nvs.read("wifi_pwd", pwd) && !ssid.empty()) {
                ESP_LOGI("Wifi", "%s", ssid.c_str());
                ESP_LOGI("Wifi", "%s", pwd.c_str());
                connect(ssid, pwd);
            }
        }
        __wifi_init = false;
    });
#endif
    //xTimerStart(t, portMAX_DELAY);
}

bool ::Wifi::isConnected() {
    return getStatus() == State::CONNECTED_GOT_IP;
}

bool _wifi_enabled = false;

bool Wifi::isEnabled() {
    return _wifi_enabled;
}
#include <soc/rtc_cntl_reg.h>

void Wifi::enable() {
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();

#ifdef USE_NVS
    if (!__wifi_init)
        CCOSCore::runOnUiThread([]() {
            bool b = true;
            CCOSCore::getNVSHandle().open().write("wifi_enabled", b);
        });
#endif
    static bool first = true;
    if (first) {
        tcpip_adapter_init();
        first = false;
    }

    esp_wifi_init(&cfg);

    esp_wifi_set_storage(WIFI_STORAGE_RAM);
    esp_wifi_set_mode(WIFI_MODE_STA);

    esp_wifi_start();
    
    _wifi_enabled = true;
}

void Wifi::disable() {

#ifdef USE_NVS
    if (!__wifi_init)
        CCOSCore::runOnUiThread([]() {
            bool b = false;
            CCOSCore::getNVSHandle().open().write("wifi_enabled", b);
        });
#endif
    esp_wifi_stop();
    esp_wifi_deinit();

    _wifi_enabled = false;
}


void Wifi::setIp(uint32_t ip) {
    _ip.b4 = static_cast<uint8_t>(ip >> 24);
    _ip.b3 = static_cast<uint8_t>(ip >> 16 & 0xff);
    _ip.b2 = static_cast<uint8_t>(ip >> 8 & 0xff);
    _ip.b1 = static_cast<uint8_t>(ip & 0xff);
}

void Wifi::setState(Wifi::State state) {
    _state = state;
    switch (_state) {
        case CONNECTED_GOT_IP:
            Cloud::onInternetConnection();

#ifdef USE_NVS
                if (!__wifi_init)
                    CCOSCore::runOnUiThread([&]() {
                        NVS::Accessor nvs = CCOSCore::getNVSHandle().open();
                        nvs.write("wifi_ssid", __ssid);
                        nvs.write("wifi_pwd", __pwd);
                    });
#endif
                xTimerStart(_checker, portMAX_DELAY);
                break;
            case State::IDLE:
            case State::DISABLED:
                xTimerStop(_checker, portMAX_DELAY);
            default:
                break;
        }
    }


    void _scan(void* callback) {
        wifi_scan_config_t scan_config;
        memset(&scan_config, 0, sizeof(wifi_scan_config_t));
        if (esp_wifi_scan_start(&scan_config, true))
            return;
        std::vector<wifi_ap_record_t> data;
        uint16_t count = 64;
        esp_wifi_scan_get_ap_num(&count);
        data.resize(count);
        esp_wifi_scan_get_ap_records(&count, data.data());
        reinterpret_cast<void (*)(const std::vector<wifi_ap_record_t> &)>(callback)(data);
        vTaskDelete(NULL);
    }

    void Wifi::scan(void (*callback)(const std::vector<wifi_ap_record_t> &)) {
        xTaskCreate(_scan, "WifiScan", 4096, (void*)callback, 2, 0);
    }

    bool Wifi::isInetAvailable() {
        return _inet;
    }
