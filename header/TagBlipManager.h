#pragma once
#include <array>
#include "TagPositions.h"

// Keeps one radar/map blip (spray can icon) alive per unsprayed tag so the player can find
// them without external maps, and draws each tag's number (1-based, matching the AP "LS Tag:
// #N" location names) over the minimap. Blips can't natively display text, so numbers only
// appear on the minimap - the pause map shows just the icons.
class TagBlipManager
{
public:
	TagBlipManager();

	// Call once per tick: creates blips for unclaimed tags, clears them once claimed, and
	// recreates any blips the game wiped (a load/new game resets the whole blip pool).
	// Returns true when that wipe was just observed - the pool reset only ever happens on a
	// real load or new game, making this the mod's authoritative "world was reloaded" signal
	// (unlike ms_LoadFileName, which also changes while merely browsing the load menu).
	bool update(const std::array<bool, 100>& t_claimed);

	// Call from the HUD draw event, after the native HUD (and radar) have drawn.
	void drawTagNumbers(const std::array<bool, 100>& t_claimed);

	// Marks one tag (0-based, -1 to clear) as the "located" tag: its blip becomes full-range,
	// so it clamps to the radar edge like a mission marker and can be followed from anywhere.
	// Driven by the client's /tag console command.
	void setLocatedTag(int t_tagIndex);

	// Player preference (pause-menu toggle): disabling clears all tag blips and stops drawing
	// numbers. The wipe-detection sentinel stays alive regardless.
	void setBlipsEnabled(bool t_enabled);

private:
	std::array<int, 100> m_blipHandles;
	int m_locatedTagIndex = -1;
	bool m_blipsEnabled = true;

	// Never-displayed blip whose death is the wipe detector - kept separate from the tag
	// blips so the signal works even if every tag is claimed.
	int m_sentinelHandle = -1;
};
