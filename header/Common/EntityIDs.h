#pragma once
#include <vector>
#include "Position.h"

inline constexpr int LOS_SANTOS_GYM_ID = 114;
inline constexpr int SAN_FIERRO_GYM_ID = 115;
inline constexpr int LAS_VENTURAS_GYM_ID = 116;
inline constexpr int TAXI_ID = 121;
inline constexpr int PARAMEDIC_ID = 122;
inline constexpr int FIREFIGHTER_ID = 123;
inline constexpr int VIGILANTE_ID = 124;
inline constexpr int BURGLARY_ID = 125;
inline constexpr int TRUCKING_ID = 117;
inline constexpr int PIMPING_ID = 127;
inline constexpr int BOAT_SCHOOL_ID = 119;
inline constexpr int BIKE_SCHOOL_ID = 120;
inline constexpr int QUARRY_ID = 118;
inline constexpr int TRAIN_ID = 126;
inline constexpr int DRIVING_SCHOOL_ID = 137;
inline constexpr int FLYING_SCHOOL_ID = 138;
inline constexpr int GANG_TERRITORY_ID = 139;

inline constexpr int EIGHT_TRACK_ID = 140;
inline constexpr int DIRT_TRACK_ID = 141;
inline constexpr int BLOOD_RING_ID = 142;
inline constexpr int KICKSTART_ID = 143;

inline constexpr int VALET_ID = 144;

inline constexpr int COURIER_LS_ID = 145;
inline constexpr int COURIER_SF_ID = 146;
inline constexpr int COURIER_LV_ID = 147;

inline constexpr int DRIVING_SCHOOL_SCORE_GLOBALS[] = {
	91, 92, 94, 96, 97, 98, 100, 101, 102, 103, 105, 107,
};
inline constexpr int FLYING_SCHOOL_SCORE_GLOBALS[] = {
	1942, 1943, 1944, 1945, 1946, 1947, 1948, 1949, 1950, 1951,
};
inline constexpr int BIKE_SCHOOL_SCORE_GLOBALS[] = {
	2164, 2165, 2166, 2167, 2168, 2169,
};
inline constexpr int BOAT_SCHOOL_SCORE_GLOBALS[] = {
	1963, 1964, 1965, 1966, 1967,
};

inline constexpr int BOAT_SCHOOL_MEDAL_GLOBALS_BASE = 1970;

inline constexpr int CHILIAD_CHALLENGE_ID = 132;
inline constexpr int CHILIAD_PASSED_GLOBAL = 1801;

inline constexpr int BLOOD_RING_PASSED_GLOBAL = 1941;
inline constexpr int KICKSTART_PASSED_GLOBAL = 90;

inline constexpr int DRIVING_SCHOOL_UNLOCKED_GLOBAL = 1397;
inline constexpr int STREET_RACES_UNLOCKED_GLOBAL = 1398;
inline constexpr int WANG_CARS_AVAILABLE_ID = 1399;
inline constexpr int RACE_PASSED_GLOBALS_BASE = 2300;

inline constexpr int RACES_LS_ID = 150;
inline constexpr int RACES_SF_ID = 151;
inline constexpr int RACES_LV_ID = 152;
inline constexpr int SHOOTING_RANGE_PISTOL_ID = 153;
inline constexpr int SHOOTING_RANGE_UZI_ID = 154;
inline constexpr int SHOOTING_RANGE_SHOTGUN_ID = 155;
inline constexpr int SHOOTING_RANGE_AK47_ID = 156;
inline constexpr int TRACE_MARKER_X_GLOBAL = 562;
inline constexpr int DRIVING_SCHOOL_BLIP_GLOBAL = 596;
inline constexpr int GARAGE_MISSIONS_GLOBAL = 541;

inline constexpr int COURIER_LS_PASSED_GLOBAL = 1992;
inline constexpr int COURIER_LV_PASSED_GLOBAL = 1993;
inline constexpr int COURIER_SF_PASSED_GLOBAL = 1994;

inline constexpr int TRAIN_LEVEL_GLOBAL = 163;
inline constexpr int TRAIN_COMPLETED_GLOBAL = 8239;

inline constexpr int COMPLETION_ID = 160;
inline constexpr int COMPLETION_AWARDED_GLOBAL = 802;

inline constexpr int OPTIONAL_MISSION_IDS[] = {
	67, 68, 69, 70,   // Wang Cars: Zeroing In, Test Drive, Customs Fast Track, Puncture Wounds

	71,               // Driving School: Back to School
	72, 73, 74,       // Zero: Air Raid, Supply Lines, New Model Army

	96, 97, 98, 99, 100, 101,

	140, 141, 142, 143,   // Stadium: 8-Track, Dirt Track, Blood Ring, Kickstart
};

struct SubmissionTierSpec
{
	int baseSlot;
	int tierCount;
	float progressPerTier;
};

inline constexpr SubmissionTierSpec PARAMEDIC_TIERS   { 0,  12, 1.0f };
inline constexpr SubmissionTierSpec FIREFIGHTER_TIERS { 12, 12, 1.0f };
inline constexpr SubmissionTierSpec VIGILANTE_TIERS   { 24, 12, 1.0f };
inline constexpr SubmissionTierSpec TAXI_TIERS        { 36, 50, 1.0f };
inline constexpr SubmissionTierSpec BURGLARY_TIERS    { 86, 10, 1000.0f };
inline constexpr SubmissionTierSpec TRUCKING_TIERS    { 96,  8, 1.0f };
inline constexpr SubmissionTierSpec VALET_TIERS       { 104,  5, 0.0f };
inline constexpr SubmissionTierSpec DRIVING_SCHOOL_TIERS { 109, 36, 1.0f };
inline constexpr SubmissionTierSpec PIMPING_TIERS     { 145, 10, 1.0f };
inline constexpr SubmissionTierSpec FLYING_SCHOOL_TIERS { 155, 30, 1.0f };
inline constexpr SubmissionTierSpec BOAT_SCHOOL_TIERS  { 185, 15, 1.0f };
inline constexpr SubmissionTierSpec BIKE_SCHOOL_TIERS  { 200, 18, 1.0f };
inline constexpr SubmissionTierSpec QUARRY_TIERS       { 218, 7, 1.0f };
inline constexpr SubmissionTierSpec GANG_TERRITORY_TIERS { 225, 20, 5.0f };
inline constexpr SubmissionTierSpec COURIER_LS_TIERS   { 245, 4, 1.0f };
inline constexpr SubmissionTierSpec COURIER_SF_TIERS   { 249, 4, 1.0f };
inline constexpr SubmissionTierSpec COURIER_LV_TIERS   { 253, 4, 1.0f };

inline constexpr SubmissionTierSpec RACES_LS_TIERS      { 257, 9, 1.0f };
inline constexpr SubmissionTierSpec RACES_SF_TIERS      { 266, 6, 1.0f };
inline constexpr SubmissionTierSpec RACES_LV_TIERS      { 272, 10, 1.0f };

inline constexpr SubmissionTierSpec SHOOTING_RANGE_PISTOL_TIERS  { 282, 1, 3.0f };
inline constexpr SubmissionTierSpec SHOOTING_RANGE_UZI_TIERS     { 283, 1, 3.0f };
inline constexpr SubmissionTierSpec SHOOTING_RANGE_SHOTGUN_TIERS { 284, 1, 3.0f };
inline constexpr SubmissionTierSpec SHOOTING_RANGE_AK47_TIERS    { 285, 1, 3.0f };

inline constexpr SubmissionTierSpec TRAIN_TIERS{ 286, 2, 1.0f };

inline constexpr int SUBMISSION_TIER_SLOT_COUNT = 288;

inline std::vector<Position> missionStartPos = {
    { 700, -3328, 20, 180 },
    { 2459.55f, -1687.75f, 12.56f, 0 },
    { 2515.0701f, -1673.98f, 12.71f, 0 },
    { 2486.6101f, -1649.42f, 12.48f, 0 },
    { 1365.2507f, -1280.12f, 12.5469f, 0 },
    { 1042.85f, -1338.62f, 12.55f, 0 },
    { 2070.8701f, -1703.01f, 12.55f, 0 },
    { 790.54f, -1627.91f, 12.39f, 0 },
    { 1801.08f, -2117.9199f, 12.56f, 0 },
    { -2043.34f, -2525.99f, 29.62f, 0 },
    { 681.595f, -478.7909f, 15.3281f, 0 },
    { 868.3358f, -29.5529f, 62.3276f, 0 },
    { -2198.8696f, -2261.2024f, 29.6419f, 0 },
    { -922.5121f, -1719.3951f, 76.5703f, 0 },
    { -513.9356f, -188.314f, 77.4599f, 0 },
    { -2030.4019f, 148.8279f, 27.8359f, 0 },
    { -2154.208f, 645.3251f, 51.3516f, 0 },
    { -1717.05f, 1280.91f, 6.23f, 0 },
    { -2623.4971f, 1405.6602f, 6.1016f, 0 },
    { -2031.2612f, 179.2488f, 27.8359f, 0 },
    { -2031.4f, -116.5f, 1034.1f, 0 },
    { -2245.6631f, 128.8889f, 34.3203f, 0 },
    { -685.2156f, 923.2191f, 11.1531f, 0 },
    { 327.448f, 2530.095f, 15.8066f, 0 },
    { 415.55f, 2533.5701f, 19.18f, 0 },
    { 1962.4316f, 974.675f, 993.4688f, 0 },
    { 2026.6028f, 1007.7353f, 9.8127f, 0 },
    { 2270.6335f, 1635.5992f, 1007.3672f, 0 },
    { 1598.5573f, 2667.8296f, 9.8203f, 0 },
    { -378.75f, 2235.8501f, 41.42f, 0 },
    { 2090.0f, 1451.0f, 9.8f, 0 },
    { 1253.788f, -785.2594f, 91.0313f, 0 },
    { 1552.7803f, 39.3031f, 23.1445f, 0 },
    { 2495.2144f, -1687.0298f, 12.5144f, 0 },
};
