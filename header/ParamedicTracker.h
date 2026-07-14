#pragma once
#include "SubmissionTracker.h"
#include <CWorld.h>

class ParamedicTracker : public SubmissionTracker
{
public:
	ParamedicTracker(int t_submissionID);
	void enforceSubmissionReward() override;
};

