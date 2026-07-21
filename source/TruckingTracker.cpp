#include "TruckingTracker.h"

TruckingTracker::TruckingTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, TRUCKING_TIERS)
{
}

void TruckingTracker::enforceSubmissionReward()
{
	// Nothing to enforce - see the note in TruckingTracker.h.
}

float TruckingTracker::getProgress() const
{
	return CStats::GetStatValue(STAT_NUMBER_OF_TRUCK_MISSIONS_PASSED);
}
