#pragma once
#include "TieredSubmissionTracker.h"

class BurglaryTracker : public TieredSubmissionTracker
{
public:
	BurglaryTracker(int t_submissionID);
	void enforceSubmissionReward() override;
	bool isVehicleValid(int t_modelId) const override;

protected:
	float getProgress() const override;
};
