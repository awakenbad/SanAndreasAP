#include "FirefighterTracker.h"

FirefighterTracker::FirefighterTracker(int t_submissionID)
	: SubmissionTracker(t_submissionID)
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
