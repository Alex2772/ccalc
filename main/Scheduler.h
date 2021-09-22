//
// Created by alex2772 on 02.09.18.
//


#pragma once


#include "Application.h"

class Scheduler: public Application {
public:
    void launch() override;

    const std::string getTitle() override;

    Bitmap getIcon() override;
};