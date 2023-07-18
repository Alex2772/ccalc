//
// Created by alex2772 on 30.04.18.
//


#pragma once


#include "OwnBitmap.h"
#include "File.h"

class BMPBitmap: public OwnBitmap {
public:
    BMPBitmap(File& f);
    BMPBitmap(const OwnBitmap& other);
    void save(File& f);
};