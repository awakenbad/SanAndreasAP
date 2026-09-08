#include "StuntJumpTracker.h"

#include <CRadar.h>
#include <CStats.h>
#include <CStuntJumpManager.h>
#include <Patch.h>
#include <common.h>

#include "StuntJumpPositions.h"

namespace
{
	constexpr uintptr_t INCREMENT_STUNT_JUMPS_STAT = 0x49C56E;
	static inline CStuntJump* lastCompletedStuntJump;

	void onStuntJumpsStatIncrease(int statID, float value)
	{
		CStats::IncrementStat(statID, value);
		lastCompletedStuntJump = CStuntJumpManager::mp_Active;
	}

	CVector getBoundingBoxCenter(CBoundingBox box)
	{
		// Calculate the center of a bounding box
		return box.m_vecMin + (box.m_vecMax - box.m_vecMin) / 2;
	}
}

StuntJumpTracker::StuntJumpTracker()
	: Collectible<70>(stuntJumpStartPositions, RADAR_SPRITE_RUNWAY, "stunt_jumps_completed", "STUNT_JUMP")
{
	plugin::patch::RedirectCall(INCREMENT_STUNT_JUMPS_STAT, &onStuntJumpsStatIncrease);
}

float StuntJumpTracker::readCount() const
{
	return CStats::GetStatValue(eStats::STAT_UNIQUE_JUMPS_DONE);
}

int StuntJumpTracker::identifyCollected() const
{
	if (!lastCompletedStuntJump) return -1;

	CPlayerPed* player = FindPlayerPed();
	if (!player) return -1;

	int best = -1;
	float bestDistance = 0.0f;
	for (int i = 0; i < static_cast<int>(stuntJumpStartPositions.size()); ++i)
	{
		// Compare the collection against the start position
		float distance = CVector::Distance(getBoundingBoxCenter(lastCompletedStuntJump->start), stuntJumpStartPositions[i]);
		if (best == -1 || distance < bestDistance)
		{
			best = i;
			bestDistance = distance;
		}
	}
	return best;
}

bool StuntJumpTracker::update()
{
	CStuntJumpManager::m_bActive = isUnlocked();

	return Collectible<70>::update();
}

CRGBA StuntJumpTracker::getHeightIndicatorColor() const
{
	return CRGBA(30, 170, 240, 255);
}
