//
// Created by alex2772 on 12.12.17.
//


#pragma once


#include "ViewIcon.h"

class MemoryIcon: public ViewIcon {
public:
    void render(Framebuffer &framebuffer) override;
};