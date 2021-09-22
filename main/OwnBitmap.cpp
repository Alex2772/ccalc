//
// Created by alex2772 on 30.04.18.
//

#include <cstring>
#include "OwnBitmap.h"

OwnBitmap::OwnBitmap(uint16_t x, uint16_t y): Bitmap(nullptr, x, y) {
    size_t s = countSize();
    if (s) {
        data = new uint8_t[s];
        memset((void *) data, 0, s);
    }
}

size_t OwnBitmap::countSize() {
    return static_cast<size_t>(width * height) / 8;
}

OwnBitmap::~OwnBitmap() {
    delete[] data;
}

OwnBitmap::OwnBitmap(const OwnBitmap &o): Bitmap(nullptr, o.width, o.height) {
    size_t s = countSize();
    if (s) {
        data = new uint8_t[s];
        memcpy((void *) data, o.data, s);
    }
}
