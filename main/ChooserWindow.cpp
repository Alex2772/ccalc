//
// Created by alex2772 on 05.12.18.
//

#include "ChooserWindow.h"

ChooserWindow::ChooserWindow(const std::string &name, const std::vector<ChooserWindow::Item> &items):
    Window(name), mItems(items)
{

}

void ChooserWindow::render(Framebuffer &framebuffer) {
    Window::render(framebuffer);
    framebuffer.shade();

    const int height = 35;

    mScroll += (mItem * height - mScroll) * 0.6f;

    for (size_t i = 0; i < mItems.size(); ++i) {
        Item& cur = mItems[i];
        int16_t y = static_cast<int16_t>(12 - mScroll + i * height);

        framebuffer.drawImageWithBorder(4, y, cur.mIcon);
        framebuffer.drawString(4 + 30 + 3, y + 8, cur.mName, OLED_COLOR_WHITE, FONT_FACE_TERMINUS_10X18_ISO8859_1, true);
    }
}

void ChooserWindow::keyDown(uint8_t key) {
    Window::keyDown(key);
    switch (key) {
        case 1:
            if (mItem == 0) {
                mItem = mItems.size() - 1;
            } else {
                --mItem;
            }
            break;
        case 9:
            ++mItem;
            mItem %= mItems.size();
            break;
        case 5:
            mChose = mItem;
            close();
    }
}

bool ChooserWindow::isTransparent() {
    return true;
}

bool ChooserWindow::hasTransparency() {
    return true;
}

ChooserWindow::~ChooserWindow() {
    if (mChose >= 0) {
        mItems[mChose].mOnSelected();
    }
}
