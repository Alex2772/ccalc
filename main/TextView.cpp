//
// Created by alex2772 on 04.12.17.
//

#include "TextView.h"

void TextView::render(Framebuffer &fb) {
    View::render(fb);
    char* cur = &(mText[0]);

    int kx = icon.data ? icon.width + 2 : 0;
    bool highlight = isFocused();
    if (highlight) {
        fb.drawRect(0, 0, width, height, OLED_COLOR_WHITE);
    }
    if (icon.data) {
        fb.drawImage(3, static_cast<int16_t>((height - icon.height) / 2), icon, false, highlight ? OLED_COLOR_BLACK : OLED_COLOR_WHITE);
    }
    int16_t i = 0;
    for (; *cur; i++)
        cur = fb.drawStringML(kx, i * 12 + 1, cur, width - x - kx, FONT_FACE_TERMINUS_6X12_ISO8859_1, highlight ? OLED_COLOR_BLACK : OLED_COLOR_WHITE);
    mMinHeight = (i + 1) * 12 + 1;
}

int TextView::getMinHeight() {
    return mMinHeight ? mMinHeight : View::getMinHeight();
}
