#include "TaxiTracker.h"

TaxiTracker::TaxiTracker(int t_submissionID)
	: SubmissionTracker(t_submissionID)
{
}

void TaxiTracker::enforceSubmissionReward()
{
	if (checkReceived)
	{
		CCheat::m_aCheatsActive[CHEAT_ALL_TAXIS_NITRO] = true;
	}
	else if (submissionCompleted)
	{
		// Unlike the set-once player fields the other trackers manage, vanilla re-asserts this
		// cheat flag continuously once 50 fares are reached, so suppressing the unearned reward
		// has to be held every tick - a one-shot revert gets overwritten again.
		CCheat::m_aCheatsActive[CHEAT_ALL_TAXIS_NITRO] = false;
	}
}
