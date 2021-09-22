//
// Created by alex2772 on 04.12.17.
//


#pragma once


#include "View.h"
#include <string>

class TextView: public View {
private:
    std::string mText;
    int mMinHeight = 0;
public:
    Bitmap icon = Bitmap(nullptr, 0, 0);
    TextView() {
        width = 128;
        height = 14;
    }

    TextView(std::string t, int16_t _x, int16_t _y):
            mText(t)
    {
        x = _x;
        y = _y;
        width = 128;
        height = 14;
    }
    virtual void render(Framebuffer& fb);
    void setText(const std::string& m) {
        mText = m;
    }
    const std::string& getText() {
        return mText;
    }

    virtual int getMinHeight();

};