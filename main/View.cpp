//
// Created by alex2772 on 02.12.17.
//

#include "View.h"
#include "CCOSCore.h"

View::View(int16_t _x, int16_t _y, int16_t _w, int16_t _h):
    x(_x),
    y(_y),
    width(_w),
    height(_h)
{

}

void View::render(Framebuffer &framebuffer) {
    if (parent) {
        framebuffer.setCoordsOffset(parent->x + x, parent->y + y);
    }
    else
        framebuffer.setCoordsOffset(x, y);
}

bool View::isFocused() {
    return parent && parent->mFocus == this;
}

void View::focus() {
    if (parent) {
        parent->setFocus(this);
    }
}

View::~View() {
    if (isFocused()) {
        parent->mFocus = nullptr;
    }
}

void View::keyDown(uint8_t key) {

}

void View::keyRelease(uint8_t key) {

}

void View::keyPressureChanged(uint8_t key) {

}

void View::keyLongDown(uint8_t key) {

}

void View::focusLost() {

}

void View::setVisibility(View::Visibility v) {
    visibility = v;
}

View::Visibility View::getVisibility() {
    return visibility;
}

void View::onClick() {

}

bool View::isTransparent() {
    return false;
}

void View::setFocus(View *focus) {

}

int View::getMinHeight() {
    return height;
}
