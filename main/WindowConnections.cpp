//
// Created by alex2772 on 14.03.19.
//

#include "WindowConnections.h"
#include "CCOSCore.h"

WindowConnections::WindowConnections(const char *t)
        : WindowList(t) {
}

void WindowConnections::keyLongDown(uint8_t key) {
    WindowList::keyLongDown(key);
    if (key == 3 && CCOSCore::getKeyState(12) == 0) {
        setEnabled(!isEnabled());
        if (!isEnabled()) {
            views.clear();
        }
    }

}

void WindowConnections::render(Framebuffer &fb) {
    bool enabled = this->isEnabled();
    if (enabled) {
        WindowList::render(fb);
    } else {
        Window::render(fb);
        std::string s = std::string("Enable ") + mTitle;
        fb.drawString(64 - s.length() * 3, 15, s.c_str());
        fb.drawString(28, 28, "by longpress");
        fb.drawString(35, 41, "backspace.");
    }
}
