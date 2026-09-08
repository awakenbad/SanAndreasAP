#pragma once

#include <Other.h>

struct EdgeTriggeredKey
{
	int vk;
	bool wasDown = false;

	bool justPressed()
	{
		bool down = plugin::KeyPressed(vk);
		bool pressed = down && !wasDown;
		wasDown = down;
		return pressed;
	}

	void prime()
	{
		wasDown = plugin::KeyPressed(vk);
	}
};
