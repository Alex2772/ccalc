//
// Created by alex2772 on 14.01.18.
//


#pragma once


#include "Application.h"
#include "Framebuffer.h"

class Today: public Application {
private:
    time_t time = 0;
    bool gen = false;
    uint8_t _bitmap[30 * 4];
public:
    void launch() override;

    const std::string getTitle() override;

    Bitmap getIcon() override;
};