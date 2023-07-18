//
// Created by alex2772 on 02.12.17.
//

#include <cmath>
#include <cstdlib>
#include "SpinnerView.h"

SpinnerView::SpinnerView(int16_t _x, int16_t _y) : View(_x, _y, 9, 9) {

}

void SpinnerView::render(Framebuffer &framebuffer) {
    View::render(framebuffer);

    angle += 7;
    angle = fmod(angle, 360);

    for (int i = 0; i < abs(androidStyleAnimation) + 1; i++) {
        float finalAngle = angle + i * 45;

        double sn = sin(-finalAngle / 3.14 / 18);
        double csn = cos(-finalAngle / 3.14 / 18);

        framebuffer.drawLine(static_cast<int8_t>(round(radius + sn * (radius * 3 / 4))),
                             static_cast<int8_t>(round(radius + csn * (radius * 3 / 4))),
                             static_cast<int8_t>(round(radius + sn * radius)),
                             static_cast<int8_t>(round(radius + csn * radius)), OLED_COLOR_WHITE);
    }
    androidStyleAnimationTicker++;
    if (androidStyleAnimationTicker > ((androidStyleAnimation == 0 || androidStyleAnimation == -6) ? 6 : 2)) {
        androidStyleAnimationTicker = 0;
        if (androidStyleAnimation < 0) {
            angle += 45;
        }
        androidStyleAnimation++;
        if (androidStyleAnimation > 6) {
            androidStyleAnimation = -6;
        }
    }
}

