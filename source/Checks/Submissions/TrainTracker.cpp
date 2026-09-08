#include "TrainTracker.h"

#include "ScriptGlobals.h"

namespace
{
	constexpr int LEVEL_ONE_PASSED = 2;
}

TrainTracker::TrainTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, TRAIN_TIERS)
{
}

void TrainTracker::enforceSubmissionReward()
{
}

float TrainTracker::getProgress() const
{
	return static_cast<float>(ScriptGlobals::read(TRAIN_LEVEL_GLOBAL));
}

int TrainTracker::currentTier() const
{
	if (ScriptGlobals::read(TRAIN_COMPLETED_GLOBAL) != 0) return 2;

	return static_cast<int>(getProgress()) >= LEVEL_ONE_PASSED ? 1 : 0;
}

bool TrainTracker::isVehicleValid(int t_modelId) const
{
	return t_modelId == MODEL_FREIGHT
		|| t_modelId == MODEL_STREAK;
}
