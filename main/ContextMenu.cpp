//
// Created by alex2772 on 14.04.18.
//

#include "ContextMenu.h"
#include "TextView.h"
#include "CCOSCore.h"

ContextMenu::Item::Item(const std::string &name, const std::function<void()> &callable) : name(name),
                                                                                          callable(callable) {}

ContextMenu::ContextMenu(const std::string &s, const std::vector<ContextMenu::Item> &items) : WindowList(s),
                                                                                              items(items) {

    for (auto& item : ContextMenu::items) {
        TextView *t = new TextView(item.name, 0, 0);
        t->width = 88;
        addView(t);
    }
    if (!views.empty()) {
        views[0]->focus();
    }
    x = 50;
}

void ContextMenu::itemSelected(size_t index) {
    WindowList::itemSelected(index);
    if (items.size() > index) {
        items[index].callable();
    }
    CCOSCore::removeWindow(this);
}

void ContextMenu::render(Framebuffer &fb) {
    View::render(fb);
    fb.drawRect(0, 0, 88, 64, OLED_COLOR_BLACK);
    WindowList::render(fb);
    fb.drawLine(-1, 0, -1, 64, OLED_COLOR_WHITE);
    fb.drawLine(-2, 0, -2, 64, OLED_COLOR_BLACK);
}

bool ContextMenu::isTransparent() {
    return true;
}

bool ContextMenu::isAnimationHorizontal() {
    return true;
}

ContextMenu::ContextMenu(std::string s): WindowList(s) {
    x = 50;
}