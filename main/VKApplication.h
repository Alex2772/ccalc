//
// Created by alex2772 on 18.11.18.
//


#pragma once


#include "Application.h"
#include "NVS.h"

class VKApplication: public Application {
public:
    void launch() override;

    const std::string getTitle() override;

    Bitmap getIcon() override;

};