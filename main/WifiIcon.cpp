//
// Created by alex2772 on 09.12.17.
//

#include <cmath>
#include <esp_wifi_types.h>
#include "WifiIcon.h"
#include "wifi.png.h"
#include "Wifi.h"

void WifiIcon::render(Framebuffer &framebuffer) {
    View::render(framebuffer);

    if (Wifi::isEnabled()) {
        Wifi::State status = Wifi::getStatus();

        switch (status) {
            case Wifi::CONNECTED_GOT_IP: {
                wifi_ap_record_t ap;
                esp_wifi_sta_get_ap_info(&ap);
                for (uint8_t i = 0; i <= uint8_t(ceilf(float(50.f - ap.rssi) / 5.f)) && i < 4; i++) {
                    Bitmap b(wifis[i], 9, 7);
                    framebuffer.drawImageWithBorder(0, 0, b, OLED_COLOR_WHITE);
                }

                if(!Wifi::isInetAvailable()) {
                    framebuffer.drawString(8, 1, "!", OLED_COLOR_WHITE, FONT_FACE_BITOCRA_4X7, true);
                }
                break;
            }
            case Wifi::CONNECTED:
            case Wifi::CONNECTING: {
                Bitmap b(wifis[_connect_anim / 8], 9, 7);
                framebuffer.drawImageWithBorder(0, 0, b, OLED_COLOR_WHITE);

                _connect_anim++;
                _connect_anim %= 32;
                break;
            }
            default:
                if (!Wifi::isConnected()) {
                    static uint8_t _connect_anim2 = 0;
                    Bitmap b(wifis[_connect_anim2 / 64], 9, 7);
                    framebuffer.drawImageWithBorder(0, 0, b, OLED_COLOR_WHITE);

                    _connect_anim2++;
                    break;
                }
                width = 0;
                return;
        }
        width = 11;
    } else {
        width = 0;
    }
}

WifiIcon::WifiIcon() {
    width = 11;
}
