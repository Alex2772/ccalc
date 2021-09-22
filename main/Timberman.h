#pragma once
#include "GameEngine.h"
#include "timberman.png.h"
#include "timberman.root.png.h"
#include "timberman.log1.png.h"
#include "timberman.log2.png.h"
#include <deque>
#include <ctime>
#include "timberman.log0.png.h"
#include "timber1.png.h"
#include "timber3.png.h"
#include "timber4.png.h"
#include "CCOSCore.h"

#ifndef min
float min(float a, float b)
{
	if (a < b)
	{
		return a;
	}
	return b;
}
#endif

class TimbermanWindow : public GameEngine
{
private:
	size_t counter = 0;
	float timer = 250;
	void drawRoundedRect(int16_t x, int16_t y, int16_t width, int16_t height, Framebuffer& fb)
	{
		fb.drawRect(x + 1, y + 1, width - 2, height - 2, OLED_COLOR_BLACK);
		fb.drawLine(x + 1, y, x + width - 1, y, OLED_COLOR_WHITE);
		fb.drawLine(x + width, y + 1, x + width, y + height - 1, OLED_COLOR_WHITE);
		fb.drawLine(x + 1, y + height, x + width - 1, y + height, OLED_COLOR_WHITE);
		fb.drawLine(x, y + 1, x, y + height - 1, OLED_COLOR_WHITE);
	}
	std::deque<uint8_t> log;
	uint8_t timberAnimation = 0;
	uint8_t logYoffset = 0;
	uint8_t chopAnimation = 0;
	bool side = false;
	bool kek = false;
	uint8_t kek_vibrate = 0;
	uint8_t loseTicks = 0;
public:
	TimbermanWindow()
		: GameEngine("Timberman")
	{
		resetGame();
	}
	void resetGame()
	{
		log.clear();
		log.push_back(0);
		log.push_back(0);
		loseTicks = 0;
		chopAnimation = 0;
		timberAnimation = 0;
		counter = 0;
		timer = 250;

	}
	void lost()
	{
		loseTicks = 15;
		CCOSCore::vibrate(20);
	}
	void particle(int16_t x, int16_t y)
	{
		MegaFX* p = new MegaFX();
		p->posX = x;
		p->posY = y;
		p->motionX = (float(esp_random() % 200) / 100.f - 1) * 10.0f;
		p->motionY = -(float(esp_random() % 200) / 200.f) * 4.1f - 0.9f;
		entities.push_back(p);
	}
	void particle(int16_t x, int16_t y, size_t c)
	{
		while (c)
		{
			particle(x, y);
			c--;
		}
	}
	uint8_t lastCnt = 0;
	virtual void render(Framebuffer& fb)
	{
		Window::render(fb);
		while (log.size() < 9)
		{
			uint8_t pot = rand() % 3;

			if (lastCnt == 0 || lastCnt == pot)
			{
				log.push_back(pot);
				lastCnt = pot;
			}
			else
			{
				log.push_back(0);
				lastCnt = 0;
			}
		}

		fb.drawRect(0, 0, width, height, OLED_COLOR_BLACK);
		static Bitmap img_root(timberman_root, 64, 5);
		fb.drawImage(32, 59, img_root, false, OLED_COLOR_WHITE);

		uint8_t yOffset = 0;


		for (std::deque<uint8_t>::iterator it = log.begin(); it != log.end(); it++)
		{
			const uint8_t* p;
			switch (*it)
			{
				default:
			case 0:
				p = timberman_log0;
				break;
			case 1:
				p = timberman_log1;
				break;
			case 2:
				p = timberman_log2;
				break;
			}
			Bitmap b(p, 64, 20);
			fb.drawImage(32, 59 - (yOffset + 1) * 18 - logYoffset / 2, b);
			if (logYoffset)
			{
				logYoffset--;

			}
			yOffset++;
		}

		char buf[16];
		sprintf(buf, "%u", counter);
		drawRoundedRect(87, 7, 40, 5, fb);
		fb.drawRect(89, 9, int16_t(36 * float(timer) / 500.f), 2, OLED_COLOR_WHITE);
		fb.drawString(0, 0, buf, OLED_COLOR_WHITE, FONT_FACE_TERMINUS_10X18_ISO8859_1);

		const uint8_t* pp;
		unsigned char offset = 4;
		if (loseTicks)
		{
			pp = timber4;
			if (loseTicks > 1)
				loseTicks--;
		}
		else
		if (chopAnimation)
		{
			pp = timber3;
			chopAnimation--;
		}
		else if (timberAnimation % 64 / 32)
		{
			pp = timber1;
			offset--;
		}
		else
		{
			pp = timber1;
		}
		if (pp == timber4)
		{
			Bitmap b(pp, 50, 40);
			if (side)
			{
				fb.drawImage(55, 24, b, false);
			}
			else
			{
				fb.drawImage(10, 24, b, false);
			}
		}
		else
		{
			Bitmap b(pp, 50, 28 - offset);
			if (side)
			{
				fb.drawImage(55, 36 + offset, b, true);
			}
			else
			{
				fb.drawImage(10, 36 + offset, b, false);
			}
		}
		if (!loseTicks)
		{
			
			timer -= min(float(counter) / 10.f, 3.f);
			if (timer < 0)
			{
				lost();
			}
		}

		if (kek)
		{
			uint8_t str = CCOSCore::getKeyState(7);
#ifdef CCOS_EMU
			str *= 40;
#endif
			particle(64, 32, str);
			kek_vibrate += str;
			if (kek_vibrate > 10)
			{
				CCOSCore::vibrate(10);
				kek_vibrate = 0;
			}
		}

		/**/
		GameEngine::render(fb);

		timberAnimation++;
	}

	virtual void keyDown(uint8_t key)
	{
		if (loseTicks == 1)
		{
			resetGame();
			return;
		}
		else if (loseTicks == 0)
		{
			if (chopAnimation)
				return;
			switch (key)
			{
			case 4:
				timberAnimation = 0;
				chopAnimation = 4;
				logYoffset = 40;
				if (log.front() == 1)
				{
					lost();
				}
				log.pop_front();
				CCOSCore::vibrate(5);
				counter++;
				timer += 20;
				if (timer > 500)
				{
					timer = 500;
				}
				side = false;
				if (log.front() == 1)
				{
					lost();
				}
				particle(55, 48, 15);
				break;
			case 6:
				timberAnimation = 0;
				chopAnimation = 4;
				logYoffset = 40;
				if (log.front() == 2)
				{
					lost();
				}
				log.pop_front();
				CCOSCore::vibrate(5);
				counter++;
				timer += 20;
				if (timer > 500)
				{
					timer = 500;
				}
				side = true;
				if (log.front() == 2)
				{
					lost();
				}
				particle(75, 48, 15);
				break;
			case 7:
				kek = true;
				break;
			}
		}
	}
	virtual void keyRelease(uint8_t c)
	{
		switch (c)
		{
		case 7:
			kek = false;
			break;
		}
	}
};

class Timberman : public Application {
public:
	virtual void launch()
	{
		CCOSCore::displayWindow(new TimbermanWindow);
	}
	virtual Bitmap getIcon() {
		return {timberman, 30, 30};
	}
	virtual const std::string getTitle() {
		return "Timberman";
	}
};
