#include "ParamedicTracker.h"

#include <CStats.h>
#include <CWorld.h>
#include <eModelID.h>

ParamedicTracker::ParamedicTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, PARAMEDIC_TIERS)
{
}

void ParamedicTracker::enforceSubmissionReward()
{
	if (checkReceived)
	{
		CWorld::Players[0].m_nMaxHealth = 176;
	}
	else if (submissionCompleted)
	{
		CWorld::Players[0].m_nMaxHealth = 100;
	}
}

float ParamedicTracker::getProgress() const
{
	return CStats::GetStatValue(STAT_HIGHEST_PARAMEDIC_MISSION_LEVEL);
}

bool ParamedicTracker::isVehicleValid(int t_modelId) const
{
	return t_modelId == MODEL_AMBULAN;
}
