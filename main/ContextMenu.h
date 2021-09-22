//
// Created by alex2772 on 14.04.18.
//


#pragma once


#include <functional>
#include "WindowList.h"

class ContextMenu: public WindowList {
public:
    class Item {
    public:
        std::string name;
        std::function<void()> callable;
        Item(const std::string &name, const std::function<void()> &callable);

    };

    ContextMenu(const std::string &s, const std::vector<Item> &items);
    ContextMenu(std::string s);

    virtual void render(Framebuffer &fb);

    virtual void itemSelected(size_t index);

    virtual bool isTransparent();

    virtual bool isAnimationHorizontal();

private:
    std::vector<Item> items;
};