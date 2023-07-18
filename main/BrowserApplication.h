//
// Created by alex2772 on 30.11.18.
//


#pragma once


#include "Application.h"

class BrowserApplication: public Application {
public:
    void launch() override;

    const std::string getTitle() override;

    Bitmap getIcon() override;
};
