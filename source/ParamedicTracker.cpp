#include "ParamedicTracker.h"

ParamedicTracker::ParamedicTracker(int t_submissionID)
	: SubmissionTracker(t_submissionID)
{
}

void ParamedicTracker::enforceSubmissionReward()
{
	if (checkReceived && !submissionCompleted)
	{
		CWorld::Players[0].m_nMaxHealth = 176;
	}
	if (!checkReceived && submissionCompleted)
	{
		CWorld::Players[0].m_nMaxHealth = 100;
	}
}
