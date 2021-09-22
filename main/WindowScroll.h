//
// Created by alex2772 on 08.03.19.
//


#pragma once


#include "Window.h"

class WindowScroll: public Window {
public:
    WindowScroll(const std::string &s);

    virtual void render(Framebuffer& fb);
    int getTotalHeight();

    int scroll = 0;
    float scrollSmooth = 0.f;

    float getScroll();
};