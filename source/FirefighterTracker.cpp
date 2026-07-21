#include "FirefighterTracker.h"

FirefighterTracker::FirefighterTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, FIREFIGHTER_TIERS)
{
}

void FirefighterTracker::enforceSubmissionReward()
{
	if (checkReceived && !submissionCompleted)
	{
		CWorld::Players[0].m_bFireProof = true;
	}
	if (!checkReceived && submissionCompleted)
	{
		CWorld::Players[0].m_bFireProof = false;
	}
}

float FirefighterTracker::getProgress() const
{
	return CStats::GetStatValue(STAT_HIGHEST_FIREFIGHTER_MISSION_LEVEL);
}
