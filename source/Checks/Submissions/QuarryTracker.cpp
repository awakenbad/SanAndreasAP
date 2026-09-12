#include "QuarryTracker.h"
#include "ScriptGlobals.h"

namespace
{
	constexpr int MISSIONS_COMPLETED_GLOBAL = 8171;
	constexpr int QUARRY_DONE_GLOBAL = 1493;
}

QuarryTracker::QuarryTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, QUARRY_TIERS)
{
}

void QuarryTracker::enforceSubmissionReward()
{
}

float QuarryTracker::getProgress() const
{
	if (ScriptGlobals::read(QUARRY_DONE_GLOBAL) == 1) return static_cast<float>(SPEC.tierCount);
	return static_cast<float>(ScriptGlobals::read(MISSIONS_COMPLETED_GLOBAL));
}
