

#include "GameEngine.h"

void GameEngine::render(Framebuffer& fb)
{
	for (size_t i = 0; i < entities.size();)
	{
		entities[i]->render(fb);
		entities[i]->lifetime--;
		if (entities[i]->lifetime == 0)
		{
			delete entities[i];
			entities.erase(entities.begin() + i);
		}
		else
		{
			i++;
		}
	}
}

MegaFX::MegaFX()
{
	rotation = esp_random() % 2 & 128;
}

void MegaFX::render(Framebuffer& fb)
{
	ParticleFX::render(fb);
	switch ((rotation & 128) ? (rotation & ~128) : 3 - (rotation & ~128))
	{
	case 0:
		fb.drawLine(int16_t(posX), int16_t(posY), int16_t(posX + 1), int16_t(posY + 1), OLED_COLOR_WHITE); // back slash
		break;
	case 1:
		fb.drawLine(int16_t(posX), int16_t(posY), int16_t(posX), int16_t(posY + 1), OLED_COLOR_WHITE); // |
		break;
	case 2:
		fb.drawLine(int16_t(posX + 1), int16_t(posY), int16_t(posX), int16_t(posY + 1), OLED_COLOR_WHITE); // /
		break;
	case 3:
		fb.drawLine(int16_t(posX), int16_t(posY), int16_t(posX + 1), int16_t(posY), OLED_COLOR_WHITE); // -
		break;
	}
	rotation++;
	rotation %= 4;
}
