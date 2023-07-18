//
// Created by alex2772 on 11.12.17.
//

#include <cmath>
#include "BatteryIcon.h"
#include "CCOSCore.h"

BatteryIcon::BatteryIcon() {
    width = 4;
    height = 7;
}

void _cross(Framebuffer& fb, int16_t x, int16_t y, Framebuffer::Color c) {
    fb.drawLine(x, y, x + 2, y + 2, c);
    fb.drawLine(x + 2, y, x, y + 2, c);
}

void BatteryIcon::render(Framebuffer &fb) {
    View::render(fb);
    fb.drawLine(1, 0, 2, 0, OLED_COLOR_INVERT);
    fb.drawRect(0, 1, 4, 6, OLED_COLOR_INVERT);
    fb.drawRect(1, 2, 2, 4, OLED_COLOR_INVERT);
    uint8_t battery = uint8_t(round(double(CCOSCore::getBatteryLevel()) / 20.0));
    switch (battery) {
        case 0:
            _cross(fb, 1, 5, OLED_COLOR_BLACK);
            _cross(fb, 3, 5, OLED_COLOR_BLACK);
            _cross(fb, 2, 4, OLED_COLOR_BLACK);
            _cross(fb, 2, 6, OLED_COLOR_BLACK);

            _cross(fb, 2, 5, OLED_COLOR_WHITE);
            break;
        default:
            fb.drawRect(1, 7 - battery, 2, battery - 1, OLED_COLOR_INVERT);
            break;
    }
}
