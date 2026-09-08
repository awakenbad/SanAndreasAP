#include "TagTracker.h"

#include <CRadar.h>
#include <common.h>
#include <cstdint>

#include "TagPositions.h"
#include "TagSprayBlocker.h"

TagTracker::TagTracker()
	: Collectible<100>(tagPositions, RADAR_SPRITE_SPRAY, "tags_claimed", "TAG")
{
}

float TagTracker::readCount() const
{
	return static_cast<float>(*reinterpret_cast<int32_t*>(TAGS_SPRAYED_ADDR));
}

int TagTracker::identifyCollected() const
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return -1;

	CVector playerPos = player->GetPosition();

	int best = -1;
	float bestDistance = 0.0f;
	for (int i = 0; i < static_cast<int>(tagPositions.size()); ++i)
	{
		float distance = CVector::Distance(playerPos, tagPositions[i]);
		if (best == -1 || distance < bestDistance)
		{
			best = i;
			bestDistance = distance;
		}
	}
	return best;
}

bool TagTracker::update()
{
	TagSprayBlocker::setLocked(!isUnlocked());

	return Collectible<100>::update();
}