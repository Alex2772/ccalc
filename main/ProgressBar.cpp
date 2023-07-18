//
// Created by alex2772 on 21.04.18.
//

#include "ProgressBar.h"

void ProgressBar::render(Framebuffer &framebuffer) {
    View::render(framebuffer);
    framebuffer.drawRect(0, 0, width, height, OLED_COLOR_BLACK);
    framebuffer.drawBorder(0, 0, width, height, OLED_COLOR_WHITE);

    framebuffer.drawRect(2, 2, static_cast<uint8_t>((width - 4) * (current / max)), height - 3, OLED_COLOR_WHITE);
}

ProgressBar::ProgressBar(int16_t _x, int16_t _y, int16_t _w) : View(_x, _y, _w, 6) {

}
