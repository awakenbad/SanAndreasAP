#include "VigilanteTracker.h"

#include <CStats.h>
#include <CWorld.h>
#include <eModelID.h>

VigilanteTracker::VigilanteTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, VIGILANTE_TIERS)
{
}

void VigilanteTracker::enforceSubmissionReward()
{
	if (checkReceived)
	{
		CWorld::Players[0].m_nMaxArmour = 150;
	}
	else if (submissionCompleted)
	{
		CWorld::Players[0].m_nMaxArmour = 100;
	}
}

float VigilanteTracker::getProgress() const
{
	return CStats::GetStatValue(STAT_HIGHEST_VIGILANTE_MISSION_LEVEL);
}

bool VigilanteTracker::isVehicleValid(int t_modelId) const
{
	return t_modelId == MODEL_COPCARLA
		|| t_modelId == MODEL_COPCARSF
		|| t_modelId == MODEL_COPCARVG
		|| t_modelId == MODEL_COPCARRU
		|| t_modelId == MODEL_ENFORCER
		|| t_modelId == MODEL_FBIRANCH
		|| t_modelId == MODEL_SWATVAN
		|| t_modelId == MODEL_COPBIKE;
}
