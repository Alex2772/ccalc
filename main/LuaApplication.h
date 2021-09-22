//
// Created by alex2772 on 16.12.17.
//


#pragma once


#include "Application.h"

class LuaApplication: public Application {
public:
    void launch() override;

    const std::string getTitle() override;

    Bitmap getIcon() override;
};