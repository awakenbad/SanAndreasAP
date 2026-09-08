#include "ShootingRangeTracker.h"

#include <CStats.h>

#include "EntityIDs.h"

namespace
{
	class RangeWeapon
	{
	public:
		const char* masteryName;
		unsigned short stat;
		int submissionID;
	};

	constexpr RangeWeapon RANGE_WEAPONS[] = {
		{ "Pistol",         STAT_PISTOL_SKILL,         SHOOTING_RANGE_PISTOL_ID },
		{ "Machine Pistol", STAT_MACHINE_PISTOL_SKILL, SHOOTING_RANGE_UZI_ID },
		{ "Shotgun",        STAT_SHOTGUN_SKILL,        SHOOTING_RANGE_SHOTGUN_ID },
		{ "AK-47",          STAT_AK_47_SKILL,          SHOOTING_RANGE_AK47_ID },
	};

	constexpr float NEW_GAME_WEAPON_SKILL = 0.0f;
	constexpr float MAX_WEAPON_SKILL = 1000.0f;
}

int shootingRangeSubmissionForWeapon(const std::string& t_weaponName)
{
	for (const RangeWeapon& weapon : RANGE_WEAPONS)
	{
		if (t_weaponName == weapon.masteryName) return weapon.submissionID;
	}
	return -1;
}

ShootingRangeTracker::ShootingRangeTracker(int t_submissionID, const SubmissionTierSpec& t_spec, int t_weaponIndex)
	: TieredSubmissionTracker(t_submissionID, t_spec), WEAPON_INDEX(t_weaponIndex)
{
}

float ShootingRangeTracker::getProgress() const
{
	return CStats::GetStatValue(STAT_SHOOTING_RANGE_LEVELS_PASSED) - WEAPON_INDEX * SPEC.progressPerTier;
}

void ShootingRangeTracker::enforceSubmissionReward()
{
	CStats::SetStatValue(RANGE_WEAPONS[WEAPON_INDEX].stat,
		checkReceived ? MAX_WEAPON_SKILL : NEW_GAME_WEAPON_SKILL);
}
