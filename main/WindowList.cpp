#include "WindowList.h"
#include "CCOSCore.h"

WindowList::WindowList(std::string _title):
	Window(_title)
{
}

void WindowList::render(Framebuffer& fb)
{
    if (!views.empty())
        if (!mFocus) {
            views.front()->focus();
        }
	Window::renderPre(fb);

	size_t yOffset = 0;
	int scrollTo = 0;

    bool scrToSet = false;

	for (size_t i = 0; i < views.size(); i++)
	{
		if (i == currentIndex) {
			scrollTo = yOffset - 18;
            scrToSet = true;
		}
		std::shared_ptr<View> v = views[i];
		v->y = 8 + yOffset - yScroll;
        yOffset += v->height + 1;
        if (!firstTime && scrToSet) {
            if (v->y >= 128) {
                break;
            } else if (v->y < 0) {
                continue;
            }
        } else {
            firstTime--;
        }
		v->render(fb);

		if (i + 1 != views.size())
		{
			fb.drawLine(1, v->height, 126, v->height, OLED_COLOR_WHITE);
		}
	}
    fb.setCoordsOffset(0, 0);

	yScroll += (scrollTo - yScroll) * 0.3f;

	if (yScroll < 0)
	{
		yScroll = 0;
	}
    Window::renderPost(fb);
    fb.drawRect(0, 0, 128, 9, OLED_COLOR_BLACK);
    Window::renderWindow(fb);
}

void WindowList::keyRelease(uint8_t key)
{
    Window::keyRelease(key);
    if (!views.empty()) {
        switch (key) {
            case 1:
                currentIndex--;
                if (currentIndex >= views.size()) {
                    currentIndex = views.size() - 1;
                }
                goto focus;
            case 9:
                currentIndex++;
                currentIndex %= views.size();
                goto focus;
            case 5:
                views[currentIndex]->onClick();
                this->itemSelected(currentIndex);
                break;
        }
        focus:
        if (views.empty()) {
            currentIndex = 0;
        } else {
            currentIndex %= views.size();
            views[currentIndex]->focus();
        }
    }
}

void WindowList::itemSelected(size_t index)
{
}

void WindowList::itemLongSelected(size_t index) {

}

void WindowList::keyLongDown(uint8_t key) {
    Window::keyLongDown(key);
    if (!views.empty()) {
        switch (key) {
            case 5:
                this->itemLongSelected(currentIndex);
                break;
        }
        views[currentIndex]->focus();
    }
}
