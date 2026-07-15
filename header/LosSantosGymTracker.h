#pragma once
#include "SubmissionTracker.h"
#include <CPed.h>

class LosSantosGymTracker : public SubmissionTracker
{
public:
	LosSantosGymTracker(int t_submissionID);
	void enforceSubmissionReward() override;
};
