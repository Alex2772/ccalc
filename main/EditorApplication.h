//
// Created by alex2772 on 15.04.18.
//


#pragma once


#include "Application.h"

class EditorApplication: public Application {

public:
    void launch() override;

    const std::string getTitle() override;

    Bitmap getIcon() override;

    bool accepts(const std::string &f) override;

    void openFile(const File &f) override;
};
