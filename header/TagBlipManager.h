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

	// Call the moment a world wipe (load / new game) is detected, BEFORE update(). Blips are
	// stored in the save file, so a load brings our old ones back while leaving our handles
	// dangling - recreating on top of them doubles the blip count every single load.
	void onWorldWiped();

	// Player preference (pause-menu toggle): disabling clears all tag blips and stops drawing
	// numbers. The wipe-detection sentinel stays alive regardless.
	void setBlipsEnabled(bool t_enabled);
	void toggleBlips();
	bool areBlipsEnabled() const;

private:
	// Identifies blips as ours by world position, since RADAR_SPRITE_SPRAY is also vanilla's
	// Pay 'n' Spray icon and so can't be used to tell them apart.
	int tagIndexAt(const CVector& t_pos) const;
	bool isOurTagPosition(const CVector& t_pos) const { return tagIndexAt(t_pos) >= 0; }
	bool ownsBlip(int t_handle) const;

	// Rebuilds our handle table from the blips actually present in the world: adopts the ones a
	// save restored and clears any duplicates. Makes the blip count idempotent no matter how we
	// arrived - fresh launch, in-session load, or a save that already accumulated duplicates.
	void reconcileWithPool();
	// Counted down over several ticks rather than run once, because a save's radar data may not
	// be in place on the very first tick after a load - a single early pass would find nothing
	// to clean up and duplicates from older saves would survive.
	int m_reconcileTicks = RECONCILE_TICKS;
	static constexpr int RECONCILE_TICKS = 10;

	// Only the nearest unsprayed tags get a radar trace, keeping plenty of the game's 175 free
	// for its own blips. The hysteresis band lets a tag already showing keep its blip slightly
	// past the cut-off, so one hovering on the boundary doesn't flicker as the player moves.
	static constexpr int MAX_TAG_BLIPS = 30;
	static constexpr int BLIP_HYSTERESIS = 10;

	// Returns the handle of a blip already sitting at this tag's position, or -1. Checked before
	// ever creating one, so a save's restored blips are adopted rather than duplicated no matter
	// what tick they happen to become visible on.
	int findExistingBlipForTag(int t_tagIndex) const;

	std::array<int, 100> m_blipHandles;
	int m_locatedTagIndex = -1;
	bool m_blipsEnabled = true;

	// Never-displayed blip whose death is the wipe detector - kept separate from the tag
	// blips so the signal works even if every tag is claimed.
	int m_sentinelHandle = -1;
};
