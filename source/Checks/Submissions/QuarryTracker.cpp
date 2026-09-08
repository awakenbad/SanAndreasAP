#include "QuarryTracker.h"

#include "ScriptGlobals.h"

namespace
{
	// Quarry missions completed - neither quarry stat counts them, so it comes from a script global.
	constexpr int MISSIONS_COMPLETED_GLOBAL = 8171;
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
	return static_cast<float>(ScriptGlobals::read(MISSIONS_COMPLETED_GLOBAL));
}
