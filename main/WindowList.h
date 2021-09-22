#pragma once

#include <string>
#include <deque>
#include "Window.h"

class WindowList : public Window
{
protected:
	size_t currentIndex = 0;
	float yScroll = 0;
	uint8_t firstTime = 5;
public:
	WindowList(std::string s);
	virtual void render(Framebuffer& fb);
	virtual void keyRelease(uint8_t key);

	virtual void keyLongDown(uint8_t index);

	virtual void itemSelected(size_t index);
	virtual void itemLongSelected(size_t index);
};
