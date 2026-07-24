#pragma once
#include <vector>
#include "Position.h"

// AP mission/location IDs of the six side-activity submissions (indices into CheckListener's
// missions table). Shared between check detection (CheckListener) and item receipt (Mod), which
// must always agree on them.
inline constexpr int LOS_SANTOS_GYM_ID = 114;
inline constexpr int TAXI_ID = 121;
inline constexpr int PARAMEDIC_ID = 122;
inline constexpr int FIREFIGHTER_ID = 123;
inline constexpr int VIGILANTE_ID = 124;
inline constexpr int BURGLARY_ID = 125;
inline constexpr int TRUCKING_ID = 117;

// Missions that sit inside the story ID range but are optional side content: they send their
// check like anything else, but spend no Progressive Mission, so running out never locks the
// player out of them. A flat list rather than a range because the branches aren't contiguous.
//
// MUST match OPTIONAL_MISSION_BRANCHES in the Python world's mission_list.py - an ID here but not
// there would be gated as story content it doesn't cost, and the reverse would silently charge
// the player for a mission the logic thinks is free.
inline constexpr int OPTIONAL_MISSION_IDS[] = {
	72, 73, 74,   // Zero: Air Raid, Supply Lines, New Model Army
};

// Deliberately absent: the marker in front of CJ's house (2495.214, -1687.03) - only Big
// Smoke starts there, which is the first story mission and needs zero Progressive Missions
// per the AP rules, so blocking it is never correct and only gets in the way afterwards.
// Submissions that pay out in tiers rather than once on completion. The check slot sent to the
// client is baseSlot + (tier - 1), and a tier is reached at progressPerTier * tier of whatever
// the tracker measures (levels, fares, dollars). Append new entries at the end - inserting in
// the middle renumbers every slot after it.
//
// MUST match worlds/gta_sa/submission_tier_list.py exactly.
struct SubmissionTierSpec
{
	int baseSlot;
	int tierCount;
	float progressPerTier;
};

inline constexpr SubmissionTierSpec PARAMEDIC_TIERS   { 0,  12, 1.0f };
inline constexpr SubmissionTierSpec FIREFIGHTER_TIERS { 12, 12, 1.0f };
inline constexpr SubmissionTierSpec VIGILANTE_TIERS   { 24, 12, 1.0f };
inline constexpr SubmissionTierSpec TAXI_TIERS        { 36, 10, 5.0f };
inline constexpr SubmissionTierSpec BURGLARY_TIERS    { 46, 10, 1000.0f };
inline constexpr SubmissionTierSpec TRUCKING_TIERS    { 56,  8, 1.0f };

inline constexpr int SUBMISSION_TIER_SLOT_COUNT = 64;

inline std::vector<Position> missionStartPos = {
    { 700, -3328, 20, 180 },
    { 2459.55, -1687.75, 12.56, 0 },
    { 2515.07, -1673.98, 12.71, 0 },
    // House Party's second part starts at its own Grove Street marker, not one of the shared
    // ones (hand-captured in-game; z lowered ~1m from the ped-origin readout to ground level).
    { 2486.59, -1649.52, 12.48, 0 },
    { 1365.251, -1280.12, 12.5469, 0 },
    { 1042.85, -1338.62, 12.55, 0 },
    { 2070.87, -1703.01, 12.55, 0 },
    { 790.54, -1627.91, 12.39, 0 },
    { 1801.08, -2117.92, 12.56, 0 },
    { -2043.34, -2525.99, 29.62, 0 },
    { 681.595, -478.7909, 15.3281, 0 },
    { 868.3358, -29.5529, 62.3276, 0 },
    { -2198.87, -2261.202, 29.6419, 0 },
    { -922.5121, -1719.395, 76.5703, 0 },
    { -513.9356, -188.314, 77.4599, 0 },
    { -2030.402, 148.8279, 27.8359, 0 },
    { -2154.208, 645.3251, 51.3516, 0 },
    { -1717.05, 1280.91, 6.23, 0 },
    { -2623.497, 1405.66, 6.1016, 0 },
    { -2031.261, 179.2488, 27.8359, 0 },
    { -2031.4, -116.5, 1034.1, 0 },
    { -2245.663, 128.8889, 34.3203, 0 },
    { -685.2156, 923.2191, 11.1531, 0 },
    { 327.448, 2530.095, 15.8066, 0 },
    { 415.55, 2533.57, 19.18, 0 },
    { 2028.932, 1023.292, 9.8127, 0 },
    { 2026.603, 1007.735, 9.8127, 0 },
    { 2175.412, 1681.548, 9.8203, 0 },
    { 1598.557, 2667.83, 9.8203, 0 },
    { -378.75, 2235.85, 41.42, 0 },
    { 2090.0, 1451.0, 9.8, 0 },
    { 2026.225, 1007.423, 9.8203, 0 },
    { 1253.788, -785.2594, 91.0313, 0 },
};
