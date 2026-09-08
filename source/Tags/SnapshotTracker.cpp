#include "SnapshotTracker.h"

#include <CCamera.h>
#include <CMatrix.h>
#include <CRadar.h>
#include <CStats.h>
#include <cmath>

#include "SnapshotLock.h"
#include "SnapshotPositions.h"

SnapshotTracker::SnapshotTracker()
	: Collectible<50>(snapshotPositions, RADAR_SPRITE_QMARK, "snapshots_claimed", "SNAPSHOT")
{
}

float SnapshotTracker::readCount() const
{
	return CStats::GetStatValue(STAT_SNAPSHOTS_TAKEN);
}

int SnapshotTracker::identifyCollected() const
{
	CVector camPos = TheCamera.m_mCameraMatrix.GetPosition();
	CVector camDir = TheCamera.m_mCameraMatrix.GetForward();

	float camLength = camDir.Magnitude();
	if (camLength < 0.0001f) return -1;
	camDir /= camLength;

	int best = -1;
	float bestDot = MIN_AIM_DOT;
	float bestDistance = 0.0f;

	for (int i = 0; i < static_cast<int>(snapshotPositions.size()); ++i)
	{
		if (isClaimed(i)) continue;

		CVector toTarget = snapshotPositions[i] - camPos;
		float distance = toTarget.Magnitude();
		if (distance < 0.0001f) continue;
		toTarget /= distance;

		float dot = camDir.x * toTarget.x + camDir.y * toTarget.y + camDir.z * toTarget.z;
		if (dot < bestDot) continue;

		if (best != -1 && std::fabs(dot - bestDot) < AIM_TIE_EPSILON && distance >= bestDistance)
		{
			continue;
		}

		best = i;
		bestDot = dot;
		bestDistance = distance;
	}

	return best;
}

bool SnapshotTracker::update()
{
	SnapshotLock::setLocked(!isUnlocked());

	return Collectible<50>::update();
}
