//
// Created by alex2772 on 03.12.17.
//


#pragma once


#include <cstdint>

class Bitmap {
public:
    const uint8_t* data = nullptr;
    uint16_t width, height;
    Bitmap(const uint8_t* data, uint16_t width, uint16_t height);
    virtual ~Bitmap();
    bool getPixelAt(uint16_t x, uint16_t y);
};