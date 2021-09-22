//
// Created by alex2772 on 30.04.18.
//


#pragma once


#include <cstdint>
#include <cstddef>
#include "Bitmap.h"

class OwnBitmap: public Bitmap {
protected:
    size_t countSize();

public:
    OwnBitmap(uint16_t x, uint16_t y);
    OwnBitmap(const OwnBitmap& o);
    virtual ~OwnBitmap();
};