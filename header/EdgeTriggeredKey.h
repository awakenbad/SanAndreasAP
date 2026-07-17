#pragma once
#include <plugin.h>

// plugin::KeyPressed is level-triggered (true every tick a key is held), so anything meant to
// fire once per press needs this edge detection. Call justPressed() exactly once per tick.
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
};
