#pragma once
#include "TieredSubmissionTracker.h"
#include <CStats.h>
#include "CWorld.h"

class VigilanteTracker : public TieredSubmissionTracker
{
public:
	VigilanteTracker(int t_submissionID);
	void enforceSubmissionReward() override;

protected:
	float getProgress() const override;
};

