#include "OysterTracker.h"

#include <CRadar.h>
#include <CStats.h>
#include <common.h>
#include <eModelID.h>

#include "OysterPositions.h"
#include "PickupLock.h"

OysterTracker::OysterTracker()
	: Collectible<50>(oysterPositions, RADAR_SPRITE_BOATYARD, "oysters_claimed", "OYSTER")
{
}

float OysterTracker::readCount() const
{
	return CStats::GetStatValue(STAT_OYSTERS_COLLECTED);
}

int OysterTracker::identifyCollected() const
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return -1;

	CVector playerPos = player->GetPosition();

	int best = -1;
	float bestDistance = 0.0f;
	for (int i = 0; i < static_cast<int>(oysterPositions.size()); ++i)
	{
		if (isClaimed(i)) continue;

		float distance = CVector::Distance(playerPos, oysterPositions[i]);
		if (best == -1 || distance < bestDistance)
		{
			best = i;
			bestDistance = distance;
		}
	}
	return best;
}

bool OysterTracker::update()
{
	PickupLock::setModelLocked(MODEL_CJ_OYSTER, !isUnlocked());

	return Collectible<50>::update();
}
