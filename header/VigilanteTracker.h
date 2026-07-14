#pragma once
#include "SubmissionTracker.h"
#include "CWorld.h"

class VigilanteTracker : public SubmissionTracker
{
public:
	VigilanteTracker(int t_submissionID);
	void enforceSubmissionReward() override;
};

