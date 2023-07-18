//
// Created by alex2772 on 05.02.19.
//

#include "KeytestWindow.h"
#include "CCOSCore.h"

KeytestWindow::KeytestWindow():
    Window("Keytest")
{

}

void KeytestWindow::render(Framebuffer &framebuffer) {
    Window::render(framebuffer);

    for (uint8_t i = 0; i < 17; ++i) {
        char buf[8];
        sprintf(buf, "%u", CCOSCore::getKeyState(i));
        framebuffer.drawString(static_cast<int16_t>(2 + (i % 4) * 26), static_cast<int16_t>(12 + (i / 4) * 10), buf);
    }
}
