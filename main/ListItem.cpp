//
// Created by alex2772 on 14.12.17.
//

#include "ListItem.h"

void ListItem::render(Framebuffer &fb) {
    View::render(fb);
    if (isFocused()) {
        fb.drawRect(0, 0, width, height, OLED_COLOR_WHITE);
    }

    int16_t offset = 1;

    char* cur = &(mTitle[0]);
    for (int16_t i = 0; *cur; i++) {
        cur = fb.drawStringML(0, offset, cur, width, FONT_FACE_TERMINUS_6X12_ISO8859_1,
                              isFocused() ? OLED_COLOR_BLACK : OLED_COLOR_WHITE);
        offset += 12;
    }
    cur = &(mText[0]);
    for (int16_t i = 0; *cur; i++) {
        cur = fb.drawStringML(0, offset, cur, width, FONT_FACE_BITOCRA_6X11,
                              isFocused() ? OLED_COLOR_BLACK : OLED_COLOR_WHITE);
        offset += 11;
    }
    height = offset + 2;
}
