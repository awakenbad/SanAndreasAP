#include "FirefighterTracker.h"

#include <CStats.h>
#include <CWorld.h>
#include <eModelID.h>

FirefighterTracker::FirefighterTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, FIREFIGHTER_TIERS)
{
}

void FirefighterTracker::enforceSubmissionReward()
{
	if (checkReceived)
	{
		CWorld::Players[0].m_bFireProof = true;
	}
	else if (submissionCompleted)
	{
		CWorld::Players[0].m_bFireProof = false;
	}
}

float FirefighterTracker::getProgress() const
{
	return CStats::GetStatValue(STAT_HIGHEST_FIREFIGHTER_MISSION_LEVEL);
}

bool FirefighterTracker::isVehicleValid(int t_modelId) const
{
	return t_modelId == MODEL_FIRETRUK;
}
