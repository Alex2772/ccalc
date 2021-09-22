//
// Created by alex2772 on 05.02.19.
//


#pragma once


#include "Window.h"

class KeytestWindow: public Window {
public:
    KeytestWindow();

    virtual void render(Framebuffer &framebuffer);
};