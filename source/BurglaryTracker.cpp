#include "BurglaryTracker.h"

BurglaryTracker::BurglaryTracker(int t_submissionID)
	: SubmissionTracker(t_submissionID)
{
}

void BurglaryTracker::enforceSubmissionReward()
{
	if (checkReceived && !submissionCompleted)
	{
		CWorld::Players[0].m_bDoesNotGetTired = true;
	}
	if (!checkReceived && submissionCompleted)
	{
		CWorld::Players[0].m_bDoesNotGetTired = false;
	}
}
