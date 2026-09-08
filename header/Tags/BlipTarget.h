#pragma once
#include <cfloat>
#include <climits>
#include <vector>
#include <CVector.h>
#include <CRGBA.h>

// One collectible described to CollectibleBlipsManager. The manager renders these and nothing more:
// which ones are near enough to show is decided before it ever sees them (see rankByDistance), so
// the manager does no distance work of its own.
struct BlipTarget
{
	CVector position;			// blip position
	CRGBA heightIndicatorColor; // height indicator color
	int sprite;					// radar sprite id (spray icon for tags, "?" for snapshots)
	int number;					// 1-based minimap label
	bool claimed;				// claimed entries are still listed, so the manager can reap their old blips
	bool located;				// the /tag one: always shown, and full-range so it edge-clamps like a marker
	int rank = INT_MAX;			// 0 = nearest unclaimed; INT_MAX = claimed / not ranked
	float distance = FLT_MAX;	// 2D metres to the player, for the radar-range test
};

// Fills in `rank` for every unclaimed target by distance to the player, nearest = 0; claimed
// targets stay INT_MAX. This is the only distance work in the whole blip pipeline - kept out of the
// manager on purpose, and done once across every collectible set so the nearest-N budget is shared.
void rankByDistance(std::vector<BlipTarget>& t_targets, const CVector& t_playerPos);
