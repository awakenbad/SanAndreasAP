#include "BurglaryTracker.h"

#include <CStats.h>
#include <CWorld.h>
#include <eModelID.h>

BurglaryTracker::BurglaryTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, BURGLARY_TIERS)
{
}

void BurglaryTracker::enforceSubmissionReward()
{
	if (checkReceived)
	{
		CWorld::Players[0].m_bDoesNotGetTired = true;
	}
	else if (submissionCompleted)
	{
		CWorld::Players[0].m_bDoesNotGetTired = false;
	}
}

float BurglaryTracker::getProgress() const
{
	return CStats::GetStatValue(STAT_MONEY_MADE_FROM_BURGLARY);
}

bool BurglaryTracker::isVehicleValid(int t_modelId) const
{
	return t_modelId == MODEL_BOXBURG;
}
