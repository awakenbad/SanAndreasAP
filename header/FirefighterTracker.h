#pragma once
#include "SubmissionTracker.h"
#include "CWorld.h"

class FirefighterTracker : public SubmissionTracker
{
public:
	FirefighterTracker(int t_submissionID);
	void enforceSubmissionReward() override;
};

