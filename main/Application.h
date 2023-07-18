//
// Created by alex2772 on 03.12.17.
//


#pragma once

#include <string>
#include "Bitmap.h"
#include "File.h"

class Application {
public:
    virtual void launch() = 0;
    virtual const std::string getTitle() = 0;
    virtual Bitmap getIcon() = 0;
    virtual bool accepts(const std::string& f) { return false; };
    virtual void openFile(const File& f) {};
};