#pragma once

#include <math.h>
#include <cstring>
#include "View.h"
#include "CCOSCore.h"
#include <cstdio>

template <typename T>
class SpinBox : public View
{
private:
	T value;
	void drawTriangle(int16_t x, int16_t y, bool napr, Framebuffer::Color c, Framebuffer& fb)
	{
		for (uint8_t i = 0; i < 3; i++)
		{

			uint8_t s = i;
			if (napr)
			{
				s = 2 - s;
			}

			fb.drawLine(x - s * 2, y + i, x + s * 2 + 1, y + i, c);
		}
	}
	char direction = 0;
	char ticker = 0;
public:
	T kmin = 0;
	T kmax = 1000;
	SpinBox(int16_t x, int16_t y, T kmin_, T kmax_, T value_):
			value(value_),
		kmin(kmin_),
		kmax(kmax_)
	{
		View::x = x;
		View::y = y;
		View::width = pow(2, sizeof(T)) * 7 + 2;
		View::height = 30;
		isInput = true;
	}
	SpinBox() {
		View::width = pow(2, sizeof(T)) * 7 + 2;
		View::height = 30;
	}
	virtual void render(Framebuffer& fb)
	{
		if (isFocused()) {
			if (direction != 0) {
				if (ticker == 0) {
					keyDown(direction == 1 ? 1 : 9);
				}
				ticker += CCOSCore::getKeyState(direction == 1 ? 1 : 9) * 2;
				if (ticker > 50)
					ticker = 0;
			}
		}
		fb.drawRect(x + 1, y + 1, width - 2, height - 2, OLED_COLOR_BLACK);
		fb.drawLine(x + 1, y, x + width - 1, y, OLED_COLOR_WHITE);
		fb.drawLine(x + width, y + 1, x + width, y + height - 1, OLED_COLOR_WHITE);
		fb.drawLine(x + 1, y + height, x + width - 1, y + height, OLED_COLOR_WHITE);
		fb.drawLine(x, y + 1, x, y + height - 1, OLED_COLOR_WHITE);
		char buf[16];
		sprintf(buf, "%i", value);
		fb.drawString(x + (width - strlen(buf) * 6) / 2, y + height / 3 + 1, buf);

		if (isFocused())
		{
			fb.drawRect(x + 1, y + 1, width - 1, height / 3 - 2, OLED_COLOR_WHITE);
			fb.drawRect(x + 1, y + (height / 3 * 2) + 3, width - 1, height / 3 - 2, OLED_COLOR_WHITE);
			
			drawTriangle(x + width / 2, y + 3, false, OLED_COLOR_BLACK, fb);
			drawTriangle(x + width / 2, y + height - 4, true, OLED_COLOR_BLACK, fb);
		}
		else
		{
			fb.drawLine(x + 1, y + height / 3 - 2, x + width - 1, y + height / 3 - 2, OLED_COLOR_WHITE);
			fb.drawLine(x + 1, y + (height / 3 * 2) + 3, x + width - 1, y + (height / 3 * 2) + 3, OLED_COLOR_WHITE);

			drawTriangle(x + width / 2, y + 3, false, OLED_COLOR_WHITE, fb);
			drawTriangle(x + width / 2, y + height - 4, true, OLED_COLOR_WHITE, fb);
		}
	}
	virtual void keyLongDown(uint8_t c)
	{
		switch (c)
		{
		case 1:
			direction = 1;
			break;
		case 9:
			direction = -1;
			break;
		}
	}
	virtual void keyDown(uint8_t c)
	{
		switch (c)
		{
		case 1:
			value++;
			if (value > kmax)
			{
				value = kmin;
			}
			else 
			if (value < kmin)
			{
				value = kmax;
			}
			CCOSCore::vibrate(2);
			break;
		case 9:
			if (value <= kmin)
			{
				value = kmax;
			}
			else
			if (value > kmax)
			{
				value = kmin;
			}
			else
				value--;
			CCOSCore::vibrate(2);
			break;
		}
	}
	virtual void keyRelease(uint8_t c)
	{
		if ((direction == 1 && c == 1) || (direction == -1 && c == 9))
		{
			direction = 0;
			ticker = 0;
		}
	}
	T getValue()
	{
		return value;
	}
	void setValue(T t)
	{
		value = t;
	}
	
	virtual void focusLost()
	{
		direction = 0;
	}
};
