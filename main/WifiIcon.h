//
// Created by alex2772 on 09.12.17.
//


#pragma once


#include "ViewIcon.h"

class WifiIcon: public ViewIcon {
private:
    uint8_t _connect_anim = 0;
public:
    WifiIcon();
    void render(Framebuffer &framebuffer) override;

};