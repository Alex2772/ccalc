//
// Created by alex2772 on 02.12.17.
//


#pragma once


#include "View.h"

class SpinnerView: public View {
private:
    float angle = 0;
    uint8_t androidStyleAnimationTicker = 0;
public:
    int8_t androidStyleAnimation = -6;
    int radius = 5;

    SpinnerView(int16_t _x, int16_t _y);

    void render(Framebuffer &framebuffer) override;
};