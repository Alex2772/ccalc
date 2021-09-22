//
// Created by alex2772 on 14.12.17.
//


#pragma once


#include "View.h"
#include <string>

class ListItem: public View {
private:
    std::string mTitle;
    std::string mText;
public:
    ListItem(std::string t):
    mTitle(t)
            {
            width = 128;
            }
    ListItem(std::string t, std::string te):
            mTitle(t),
            mText(te)
    {
        width = 128;
    }
    ListItem() {
        width = 128;
    }
    virtual void render(Framebuffer& fb);
    void setTitle(const std::string& m) {
        mTitle = m;
    }
    void setText(const std::string& m) {
        mText = m;
    }
    const std::string& getText() {
        return mText;
    }
};