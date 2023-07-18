//
// Created by alex2772 on 15.04.18.
//


#pragma once


#include "Window.h"

class Dialog: public Window {
public:
    int16_t xOffset = 15;
    Dialog(const std::string &s);

    void renderWindow(Framebuffer &framebuffer) override;

    bool hasTransparency() override;

};