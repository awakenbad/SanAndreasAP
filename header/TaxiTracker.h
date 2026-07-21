#pragma once
#include "TieredSubmissionTracker.h"
#include <CCheat.h>

class TaxiTracker : public TieredSubmissionTracker
{
public:
	TaxiTracker(int t_submissionID);
	void enforceSubmissionReward() override;

protected:
	// Taxi has no fare stat in eStats.h - progress comes from a raw address instead, which is
	// exactly why TieredSubmissionTracker asks for a value rather than a stat id.
	float getProgress() const override;
};
