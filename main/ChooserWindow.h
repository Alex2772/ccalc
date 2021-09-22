//
// Created by alex2772 on 05.12.18.
//


#pragma once


#include "Window.h"
#include <vector>

class ChooserWindow: public Window {
protected:
    struct Item {
        std::string mName;
        Bitmap mIcon;
        std::function<void()> mOnSelected;
    };
private:
    std::vector<Item> mItems;
    size_t mItem = 0;
    float mScroll = 0.f;
    int mChose = -1;
public:
    ChooserWindow(const std::string& name, const std::vector<Item>& items);
    ~ChooserWindow();

    virtual void render(Framebuffer &framebuffer);

    virtual bool isTransparent();

    virtual bool hasTransparency();

    virtual void keyDown(uint8_t key);
};