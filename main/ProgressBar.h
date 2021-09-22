//
// Created by alex2772 on 21.04.18.
//


#pragma once


#include "View.h"

class ProgressBar: public View {
public:
    float max = 100.f;
    float current = 0.f;

    ProgressBar(int16_t _x, int16_t _y, int16_t _w);

    virtual void render(Framebuffer &framebuffer);
};