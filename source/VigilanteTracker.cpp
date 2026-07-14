#include "VigilanteTracker.h"

VigilanteTracker::VigilanteTracker(int t_submissionID)
	: SubmissionTracker(t_submissionID)
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
