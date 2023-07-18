//
// Created by alex2772 on 03.12.17.
//

#include "Window.h"
#include "CCOSCore.h"

#include <utility>

Window::~Window() {

}

void Window::addViewS(std::shared_ptr<View> v) {
    v->parent = this;
    views.push_back(v);
}

void Window::render(Framebuffer &framebuffer) {
    renderPre(framebuffer);
    renderPost(framebuffer);
}

Window::Window(std::string s): View(0, 0, 128, 64), mTitle(std::move(s)) {

}

void Window::renderPre(Framebuffer &fb) {
    View::render(fb);
}

void Window::renderPost(Framebuffer &framebuffer) {
    View::render(framebuffer);
    for (size_t i = 0; i < views.size(); i++) {
        if (views[i]->getVisibility() == VISIBLE)
            views[i]->render(framebuffer);
    }
    this->renderWindow(framebuffer);
}
void Window::renderWindow(Framebuffer& framebuffer) {
    View::render(framebuffer);
    if (mHighlight) {
        framebuffer.drawRect(0, 0, 127, 8, OLED_COLOR_INVERT);
    }
    if (mDrawTitle) {
        framebuffer.drawString(0, 0, mTitle.c_str(), OLED_COLOR_INVERT, FONT_FACE_BITOCRA_4X7);
        framebuffer.drawLine(0, 8, 127, 8, OLED_COLOR_INVERT);
    }
}
void Window::focusNext() {
    for (size_t i = 0; i < views.size() && !views.empty(); i++) {
        focusIndex = (focusIndex + 1) % views.size();
        if (views[focusIndex]->isInput && views[focusIndex]->getVisibility() != View::GONE) {
            views[focusIndex]->focus();
            CCOSCore::vibrate(10);
            return;
        }
    }
}

void Window::onPause() {
}

void Window::onResume() {
}

void Window::keyLongDown(uint8_t key) {
    View::keyLongDown(key);
    if (mFocus && _sa == 0) {
        mFocus->keyLongDown(key);
    }
}

void Window::keyDown(uint8_t key) {
    View::keyDown(key);
    if (mFocus && _sa == 0) {
        mFocus->keyDown(key);
    }
}

void Window::keyPressureChanged(uint8_t key) {
    View::keyPressureChanged(key);
    if (mFocus && _sa == 0) {
        mFocus->keyPressureChanged(key);
    }
}

void Window::keyRelease(uint8_t key) {
    View::keyRelease(key);
    if (mFocus && _sa == 0) {
        mFocus->keyRelease(key);
    }
}

bool Window::isTransparent() {
    return _sa || this->hasTransparency();
}

bool Window::isAnimationHorizontal() {
    return false;
}

bool Window::hasTransparency() {
    return false;
}

bool Window::isFullscreen() {
    return false;
}

const std::string &Window::getTitle() {
    return mTitle;
}

void Window::addView(View* v) {
    std::shared_ptr<View> s(v);
    addViewS(s);
}

void Window::close() {
    CCOSCore::removeWindow(this);
}

bool Window::sleepLock() {
    return false;
}

void Window::event(Event e) {

}

void Window::lua_close() {

}

bool Window::isFocused() {
    return CCOSCore::getWindows().back() == this;
}

void Window::setFocus(View *focus) {
    mFocus = focus;
    for (size_t i = 0; i < views.size(); ++i) {
        if (views[i].get() == focus) {
            focusIndex = i;
            break;
        }
    }
}
