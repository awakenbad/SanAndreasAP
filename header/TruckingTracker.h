#pragma once
#include "TieredSubmissionTracker.h"
#include <CStats.h>

// RS Haul in Flint County: 8 delivery missions, one check each. Unlike the other tiered
// submissions this one has no vanilla reward worth suppressing - finishing all 8 turns the
// depot into a revenue-generating asset, which is passive money rather than a player ability -
// so there is no AP reward item and enforceSubmissionReward has nothing to do.
class TruckingTracker : public TieredSubmissionTracker
{
public:
	TruckingTracker(int t_submissionID);
	void enforceSubmissionReward() override;

protected:
	float getProgress() const override;
};
