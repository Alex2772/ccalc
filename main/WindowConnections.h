//
// Created by alex2772 on 14.03.19.
//


#pragma once


#include "WindowList.h"

class WindowConnections : public WindowList {
public:
    WindowConnections(const char *t);

    virtual bool isEnabled() = 0;

    virtual void setEnabled(bool v) = 0;

    virtual void keyLongDown(uint8_t key) override;

#ifdef KEY_EMU
    virtual void keyDown(uint8_t key) override {
        WindowList::keyLongDown(key);
        if (key == 3 && CCOSCore::getKeyState(12) == 0)
            setEnabled(!isEnabled());

    }

#endif

    virtual void render(Framebuffer &fb) override;
};