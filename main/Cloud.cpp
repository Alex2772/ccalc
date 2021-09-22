#include <freertos/FreeRTOS.h>
#include "Cloud.h"
#include "Socket.h"
#include "config.h"
#include "CCOSCore.h"
#include "OwlMeteoService.h"
#include <freertos/timers.h>
#include <cstring>
#include "FtpServer.h"

static bool syncing = false;
static bool synced = false;

void sync_timer(TimerHandle_t t) {
    Cloud::sync();
}

MeteoServiceProvider::meteo_data* __meteo_data = nullptr;

void __sync(void* p) {
    bool synced_prev = synced;
    syncing = true;
    synced = true;
    {
        Socket s("alex2772.ru", 4322);
        if (s.state == Socket::STATE_OK) {
            CCOSCore::runOnUiThread([]() {
                CCOSCore::showToast("Synced");
            });
            auto time = s.read<time_t>();
            time += TIME_OFFSET * 3600;
            CCOSCore::setTime(time);


            if (!synced_prev) {
                TimerHandle_t t = xTimerCreate("SyncTimer", 1000 * 600 / portTICK_PERIOD_MS, pdTRUE, 0, sync_timer);

                xTimerStart(t, portMAX_DELAY);
            }
        } else {
            int state = s.state;
            CCOSCore::runOnUiThread([state]() {
                char buf[16];
                sprintf(buf, "%d", state);
                CCOSCore::showToast(buf);
            });
            synced = false;
        }
    }
    OwlMeteoService owl;
    MeteoServiceProvider::meteo_data d;
    if (owl.fill(d)) {
        if (!__meteo_data)
            __meteo_data = new MeteoServiceProvider::meteo_data;
        memcpy(__meteo_data, &d, sizeof(d));
    } else {
        CCOSCore::showToast("Owl failed");
    }

    syncing = false;
    vTaskDelete(NULL);
}

void Cloud::sync() {
    xTaskCreate(__sync, "SyncTask", 4096, NULL, 2, NULL);
}

void ::Cloud::onInternetConnection() {
    if (!synced) {
        sync();
    }
    static FtpServer* _ftp;
    if (!_ftp) {
        _ftp = new FtpServer;
    }
}

bool ::Cloud::isSyncing() {
    return syncing;
}
