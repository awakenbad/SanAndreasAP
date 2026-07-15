#pragma once
#include "SubmissionTracker.h"
#include <CWorld.h>

class BurglaryTracker : public SubmissionTracker
{
public:
	BurglaryTracker(int t_submissionID);
	void enforceSubmissionReward() override;
};
