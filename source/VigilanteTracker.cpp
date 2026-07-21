#include "VigilanteTracker.h"

VigilanteTracker::VigilanteTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, VIGILANTE_TIERS)
{
}

void VigilanteTracker::enforceSubmissionReward()
{
	if (checkReceived && !submissionCompleted)
	{
		CWorld::Players[0].m_nMaxArmour = 150;
	}
	if (!checkReceived && submissionCompleted)
	{
		CWorld::Players[0].m_nMaxArmour = 100;
	}
}

float VigilanteTracker::getProgress() const
{
	return CStats::GetStatValue(STAT_HIGHEST_VIGILANTE_MISSION_LEVEL);
}
