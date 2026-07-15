#include "LosSantosGymTracker.h"
#include "common.h"

LosSantosGymTracker::LosSantosGymTracker(int t_submissionID)
	: SubmissionTracker(t_submissionID)
{
}

void LosSantosGymTracker::enforceSubmissionReward()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return;

	if (checkReceived && !submissionCompleted)
	{
		player->m_nFightingStyle = STYLE_BOXING;
	}
	if (!checkReceived && submissionCompleted)
	{
		player->m_nFightingStyle = STYLE_STANDARD;
	}
}
