#include "SnapshotTracker.h"
#include "SnapshotPositions.h"
#include "SnapshotLock.h"
#include "common.h"
#include <CStats.h>
#include <CRadar.h>
#include <Patch.h>
#include <CPickups.h>

namespace
{
	constexpr uintptr_t SNAPSHOT_REMOVE_ENTITY = 0x456C19;
	static inline CPickup* lastCompletedSnapshot;

	void onSnapshotBlipCleared(eBlipType blipType, int entityHandle)
	{
		// Inverse calculation for "(v10 - CPickups::aPickUps) | (CPickups::aPickUps[v10 - CPickups::aPickUps].m_wReferenceIndex << 16)"
		int pickupIndex = entityHandle & 0xFFFF;
		lastCompletedSnapshot = &CPickups::aPickUps[pickupIndex];

		CRadar::ClearBlipForEntity(blipType, entityHandle);
	}
}

SnapshotTracker::SnapshotTracker()
	: Collectible<50>(snapshotPositions, RADAR_SPRITE_QMARK, "snapshots_claimed", "SNAPSHOT")
{
	plugin::patch::RedirectCall(SNAPSHOT_REMOVE_ENTITY, &onSnapshotBlipCleared);
}

float SnapshotTracker::readCount() const
{
	return CStats::GetStatValue(STAT_SNAPSHOTS_TAKEN);
}

int SnapshotTracker::identifyCollected() const
{
	int best = -1;
	float bestDistance = 0.0f;

	if (!lastCompletedSnapshot) return -1;

	for (int i = 0; i < static_cast<int>(snapshotPositions.size()); ++i)
	{
		if (isClaimed(i)) continue;

		float distance = snapshotPositions[i].Distance(lastCompletedSnapshot->GetPosn());

		if (best != -1 && distance >= bestDistance) continue;

		best = i;
		bestDistance = distance;
	}

	return best;
}

bool SnapshotTracker::update()
{
	SnapshotLock::setLocked(!isUnlocked());

	return Collectible<50>::update();
}
