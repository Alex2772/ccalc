//
// Created by alex2772 on 08.03.19.
//

#include "WindowScroll.h"
#include "CCOSCore.h"

WindowScroll::WindowScroll(const std::string &s) : Window(s) {}

void WindowScroll::render(Framebuffer &fb) {
    int height = getTotalHeight();
    if (height < 64) {
        scroll = 0;
        scrollSmooth = 0;
    } else {
        scroll += (int(CCOSCore::getKeyState(9)) - int(CCOSCore::getKeyState(1))) * 2;
        scrollSmooth += (scroll - scrollSmooth) * 0.25f;

        scroll = glm::clamp(scroll, 0, height - 64);
    }
    int y = static_cast<int>(-scrollSmooth + 9);
    for (auto& p : views) {
        p->x = 2;
        p->width = 122;
        p->y = static_cast<int16_t>(y);
        y += p->y;
    }

    mFocus = nullptr;
    Window::render(fb);

    fb.drawLine(127, 12, 127, 64, OLED_COLOR_BLACK);

    int sbHeight = int(glm::max(float((64 - 12)) / float(height), 4.f));
    int sbY = scrollSmooth / height * (64 - 12) + 12;
    fb.drawLine(127, sbY, 127, sbY + sbHeight, OLED_COLOR_WHITE);
}
float WindowScroll::getScroll() {
    return scrollSmooth / (getTotalHeight() - 64);
}

int WindowScroll::getTotalHeight() {
    int height = 0;
    for (auto& p : views) {
        int h = p->getMinHeight();
        p->height = h;
        height += h;
    }
    return height;
}
