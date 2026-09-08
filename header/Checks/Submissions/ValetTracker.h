#pragma once
#include "TieredSubmissionTracker.h"

// Valet Parking at the Vank Hoff Hotel: 5 unevenly spaced levels, one check each. Its reward is
// money and an asset rather than a player ability, so there is no AP item to enforce.
class ValetTracker : public TieredSubmissionTracker
{
public:
	ValetTracker(int t_submissionID);
	void enforceSubmissionReward() override;

protected:
	float getProgress() const override;
	int currentTier() const override;
};
