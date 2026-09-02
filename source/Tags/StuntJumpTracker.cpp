#include "StuntJumpTracker.h"

#include "StuntJumpPositions.h"
#include "common.h"
#include <CRadar.h>
#include <CStats.h>
#include <CStuntJumpManager.h>

StuntJumpTracker::StuntJumpTracker()
	: Collectible<70>(stuntJumpStartPositions, RADAR_SPRITE_RUNWAY, "stunt_jumps_completed", "STUNT_JUMP")
{
}

float StuntJumpTracker::readCount() const
{
	return CStats::GetStatValue(eStats::STAT_UNIQUE_JUMPS_DONE);
}

int StuntJumpTracker::identifyCollected() const
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return -1;

	CVector playerPos = player->GetPosition();

	int best = -1;
	float bestDistance = 0.0f;
	for (int i = 0; i < static_cast<int>(stuntJumpEndPositions.size()); ++i)
	{
		// Compare the collection against the end position
		float distance = CVector::Distance(playerPos, stuntJumpEndPositions[i]);
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