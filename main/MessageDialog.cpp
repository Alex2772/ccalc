//
// Created by alex2772 on 18.04.18.
//

#include "MessageDialog.h"
#include "CCOSCore.h"

MessageDialog::MessageDialog(const std::string &s, const std::string &msg): Dialog(s), mText(msg) {
    xOffset = 0;
}
void _border(Framebuffer& fb, std::string& mText, int16_t x, int16_t y, Framebuffer::Color color) {
    char* cur = &(mText[0]);
    for (int16_t i = 0; *cur; i++)
        cur = fb.drawStringML(x, i * 12 + y, cur, 128 - 10, FONT_FACE_TERMINUS_6X12_ISO8859_1, color);
}
void MessageDialog::render(Framebuffer &fb) {
    Window::render(fb);
    fb.setCoordsOffset(0, 0);
    int16_t y = 4;
    if (mTitle.empty()) {
        y += 11;
    }
    //fb.drawRect(8, y, 128 - 5 * 2, 64 - 8 * 2 - y, OLED_COLOR_BLACK);
    //fb.drawBorder(8, y, 128 - 5 * 2, 64 - 8 * 2 - y, OLED_COLOR_WHITE);


    _border(fb, mText, 0 + -1, 17, OLED_COLOR_BLACK);
    _border(fb, mText, 0 + 1, 17, OLED_COLOR_BLACK);
    _border(fb, mText, 0, 17 - 1, OLED_COLOR_BLACK);
    _border(fb, mText, 0, 17 + 1, OLED_COLOR_BLACK);
    _border(fb, mText, 0 + -1, 17 - 1, OLED_COLOR_BLACK);
    _border(fb, mText, 0 + 1, 17 - 1, OLED_COLOR_BLACK);
    _border(fb, mText, 0 + -1, 17 + 1, OLED_COLOR_BLACK);
    _border(fb, mText, 0 + 1, 17 + 1, OLED_COLOR_BLACK);
    _border(fb, mText, 0, 17, OLED_COLOR_WHITE);

}

void MessageDialog::keyDown(uint8_t key) {
    Window::keyRelease(key);
    if (key == 15) {
        if (mCallback)
            mCallback();
        CCOSCore::removeWindow(this);
    }
}

MessageDialog::MessageDialog(const std::string &s, const std::string &msg, const std::function<void()>& m):
    MessageDialog(s, msg)
{
    mCallback = m;
}
