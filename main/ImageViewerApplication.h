//
// Created by alex2772 on 17.12.17.
//


#pragma once


#include "Application.h"

class ImageViewerApplication: public Application {
public:
    void launch() override;

    const std::string getTitle() override;

    virtual bool accepts(const std::string &f);

    virtual void openFile(const File &f);

    Bitmap getIcon() override;
};