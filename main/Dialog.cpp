//
// Created by alex2772 on 15.04.18.
//

#include "Dialog.h"

Dialog::Dialog(const std::string &s) : Window(s) {

}

bool Dialog::hasTransparency() {
    return true;
}

void Dialog::renderWindow(Framebuffer &framebuffer) {
    View::render(framebuffer);
    framebuffer.drawString(xOffset, 4, mTitle.c_str(), OLED_COLOR_WHITE, FONT_FACE_TERMINUS_8X14_ISO8859_1, true);
}
