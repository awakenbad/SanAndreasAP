#pragma once
#include "TieredSubmissionTracker.h"

class TaxiTracker : public TieredSubmissionTracker
{
public:
	TaxiTracker(int t_submissionID);
	void enforceSubmissionReward() override;
	bool isVehicleValid(int t_modelId) const override;

protected:
	float getProgress() const override;
};
