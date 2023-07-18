//
// Created by alex2772 on 05.02.19.
//


#pragma once

#include "Window.h"
#include "TaskHelper.h"
#include "TextView.h"

class PItestWindow: public Window {
private:
    TaskHelper mThread;
    double mValue = 0;
    TextView *mLabel;
    TextView *mCounterLabel;

    size_t mK = 0;
public:
    PItestWindow();

    virtual void render(Framebuffer &framebuffer);

};