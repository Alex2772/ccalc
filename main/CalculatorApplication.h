//
// Created by alex2772 on 03.12.17.
//


#pragma once


#include "Application.h"

class CalculatorApplication: public Application {
public:
    void launch() override;

    const std::string getTitle() override;

    Bitmap getIcon() override;
};