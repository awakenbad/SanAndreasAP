#pragma once
#include <CVector.h>

// The blip and sphere a branch puts up where its next mission is offered
class Marker
{
public:
	static constexpr int LEAVE_DISPLAY = -1;
	static constexpr int BLIP_ONLY = 2;

	CVector position;
	int sprite = 0;
	int blipHandleOffset = 0;
	int blipDisplay = LEAVE_DISPLAY;

	// Set when a script removes this blip without zeroing the global
	bool handleMayBeStale = false;

	void raise() const;
	void clearForeign() const;
	void clearAll() const;

private:
	void clear(bool t_includeOurSprite) const;
	bool stillDrawn() const;
};

