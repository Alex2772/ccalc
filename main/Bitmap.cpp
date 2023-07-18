//
// Created by alex2772 on 03.12.17.
//

#include "Bitmap.h"
#include <glm/glm.hpp>

Bitmap::Bitmap(const uint8_t *d, uint16_t w, uint16_t h):
    data(d),
    width(w),
    height(h)
{

}

Bitmap::~Bitmap() {

}

bool Bitmap::getPixelAt(uint16_t x, uint16_t y) {
    x = glm::clamp(x, uint16_t(0), width);
    y = glm::clamp(y, uint16_t(0), height);
    uint8_t byte = data[y / 8 * width + x];
    byte &= 1 << (y % 8);
    return byte > 0;
}

