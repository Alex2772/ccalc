//
// Created by alex2772 on 11.12.17.
//


#pragma once


#include "ViewIcon.h"

class BatteryIcon: public ViewIcon {
public:
    BatteryIcon();

    void render(Framebuffer &framebuffer) override;
};