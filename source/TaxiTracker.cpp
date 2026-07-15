#include "TaxiTracker.h"

TaxiTracker::TaxiTracker(int t_submissionID)
	: SubmissionTracker(t_submissionID)
{
}

void TaxiTracker::enforceSubmissionReward()
{
	if (checkReceived && !submissionCompleted)
	{
		CCheat::m_aCheatsActive[CHEAT_ALL_TAXIS_NITRO] = true;
	}
	if (!checkReceived && submissionCompleted)
	{
		CCheat::m_aCheatsActive[CHEAT_ALL_TAXIS_NITRO] = false;
	}
}
