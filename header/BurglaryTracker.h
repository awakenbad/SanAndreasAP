#pragma once
#include "TieredSubmissionTracker.h"
#include <CStats.h>
#include <CWorld.h>

class BurglaryTracker : public TieredSubmissionTracker
{
public:
	BurglaryTracker(int t_submissionID);
	void enforceSubmissionReward() override;

protected:
	float getProgress() const override;
};
