#pragma once
#include "TieredSubmissionTracker.h"

class VigilanteTracker : public TieredSubmissionTracker
{
public:
	VigilanteTracker(int t_submissionID);
	void enforceSubmissionReward() override;
	bool isVehicleValid(int t_modelId) const override;

protected:
	float getProgress() const override;
};

