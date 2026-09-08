#include "GlobalFlagTracker.h"

#include "ScriptGlobals.h"

GlobalFlagTracker::GlobalFlagTracker(int t_locationId, int t_completedGlobal)
	: SubmissionTracker(t_locationId), m_completedGlobal(t_completedGlobal)
{
}

void GlobalFlagTracker::enforceSubmissionReward()
{
}

bool GlobalFlagTracker::pollCompletion()
{
	if (submissionCompleted) return false;

	return ScriptGlobals::read(m_completedGlobal) == 1;
}
