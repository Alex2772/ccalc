//
// Created by alex2772 on 02.12.17.
//


#pragma once


#include <cstdint>
#include "Framebuffer.h"

class View {
public:
    enum Visibility {
        VISIBLE,
        INVISIBLE,
        GONE
    };
    View* mFocus = nullptr;

    bool isInput = false;
    int16_t x = 0;
    int16_t y = 0;
    int16_t width = 128;
    int16_t height = 64;

    View() {}
    View(int16_t _x, int16_t _y, int16_t _w, int16_t _h);
    virtual ~View();

    virtual void render(Framebuffer& framebuffer);

    virtual bool isFocused();

    virtual void focus();

    void setVisibility(Visibility v);
    Visibility getVisibility();

    virtual void keyLongDown(uint8_t key);
    virtual void keyDown(uint8_t key);
    virtual void keyPressureChanged(uint8_t key);
    virtual void keyRelease(uint8_t key);
    virtual void onClick();
    virtual bool isTransparent();

    virtual void setFocus(View* focus);

    View* parent = nullptr;

    virtual void focusLost();
    virtual int getMinHeight();

private:
    Visibility visibility = VISIBLE;
};