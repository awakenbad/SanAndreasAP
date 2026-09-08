#pragma once
#include "TieredSubmissionTracker.h"

class FirefighterTracker : public TieredSubmissionTracker
{
public:
	FirefighterTracker(int t_submissionID);
	void enforceSubmissionReward() override;
	bool isVehicleValid(int t_modelId) const override;

protected:
	float getProgress() const override;
};

