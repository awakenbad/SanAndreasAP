#pragma once
#include "SubmissionTracker.h"
#include <CCheat.h>

class TaxiTracker : public SubmissionTracker
{
public:
	TaxiTracker(int t_submissionID);
	void enforceSubmissionReward() override;
};
