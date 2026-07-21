#pragma once
#include "SubmissionTracker.h"
#include "EntityIDs.h"
#include <vector>

// Base for submissions that pay out in tiers instead of once on completion - Paramedic /
// Firefighter / Vigilante per level, Taxi per 5 fares, Burglary per $1000 stolen.
//
// Subclasses supply only where their progress comes from (getProgress), so a stat, a raw
// address or anything else works; the tier arithmetic, the sent-tier bookkeeping and the
// completion latch live here.
class TieredSubmissionTracker : public SubmissionTracker
{
public:
	TieredSubmissionTracker(int t_submissionID, const SubmissionTierSpec& t_spec);

	void pollNewTierSlots(std::vector<int>& t_outSlots) override;

	int getSentTier() const override;
	void restoreSentTier(int t_tier) override;

protected:
	// Current raw progress: levels reached, fares driven, dollars stolen...
	virtual float getProgress() const = 0;

private:
	const SubmissionTierSpec SPEC;
	int m_sentTier = 0;
};
