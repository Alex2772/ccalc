#pragma once
#include <vector>
#include "Window.h"


class Entity
{
public:
	float posX = 0;
	float posY = 0;
	float motionX = 0;
	float motionY = 0;
	uint8_t lifetime = 20;
	Entity()
	{
	}
    virtual ~Entity() = default;
	virtual void render(Framebuffer& fb)
	{
		posX += motionX;
		posY += motionY;
	}
};

class ParticleFX : public Entity
{
public:
    virtual ~ParticleFX() = default;
	virtual void render(Framebuffer& fb)
	{
		Entity::render(fb);
		motionY += 1.f;
		if (motionY > 1.f)
			motionY -= 0.1f;
		motionX *= 0.8f;
	}
};

class MegaFX : public ParticleFX
{
public:
	unsigned char rotation = 0;
	MegaFX();
	virtual void render(Framebuffer& fb);
};


class GameEngine : public Window
{
protected:
	std::vector<Entity*> entities;
public:
	virtual ~GameEngine()
	{
		for (size_t i = 0; i < entities.size(); i++)
		{
			delete entities[i];
		}
	}
	GameEngine(std::string s)
		: Window(s)
	{
	}

	virtual void render(Framebuffer& fb);
};
