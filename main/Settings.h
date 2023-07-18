#pragma once


#include "settings.png.h"
#include "Application.h"

class Settings: public Application {
public:
    virtual void launch();
    virtual Bitmap getIcon() {
        return {settings, 30, 30};
    }
    virtual const std::string getTitle() {
        return {"Settings"};
    }
};
