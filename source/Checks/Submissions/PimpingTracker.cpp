#include "PimpingTracker.h"

#include <CStats.h>
#include <eModelID.h>

PimpingTracker::PimpingTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, PIMPING_TIERS)
{
}

void PimpingTracker::enforceSubmissionReward()
{
}

float PimpingTracker::getProgress() const
{
	return CStats::GetStatValue(STAT_PIMPING_LEVEL);
}

bool PimpingTracker::isVehicleValid(int t_modelId) const
{
	return t_modelId == MODEL_BROADWAY;
}
