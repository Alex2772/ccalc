//
// Created by alex2772 on 17.12.17.
//


#pragma once


#include <functional>
#include "Bitmap.h"
#include "Framebuffer.h"

class Picture: public Bitmap {
public:
    Picture(const uint8_t *data, uint16_t width, uint16_t height);

public:
    virtual void draw(Framebuffer& fb, int16_t x, int16_t y);
};

class LivePicture: public Picture {
private:
    std::function<void(Framebuffer& fb)> _dr;
public:
    LivePicture(const uint8_t *data, uint16_t width, uint16_t height, std::function<void(Framebuffer& fb)> dr);
    virtual void draw(Framebuffer& fb, int16_t x, int16_t y);
};