//
// Created by alex2772 on 28.10.18.
//


#pragma once


#include "ViewIcon.h"

class StreamIcon: public ViewIcon {
public:
    StreamIcon();

    virtual void render(Framebuffer &framebuffer);
};