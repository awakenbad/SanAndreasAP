#pragma once
#include "SubmissionTracker.h"

#include <vector>

#include "EntityIDs.h"

class TieredSubmissionTracker : public SubmissionTracker
{
public:
	TieredSubmissionTracker(int t_submissionID, const SubmissionTierSpec& t_spec);

	void pollNewTierSlots(std::vector<int>& t_outSlots) override;

	std::string getSentState() const override;
	void restoreSentState(const std::string& t_state) override;

protected:
	virtual float getProgress() const = 0;

	virtual int currentTier() const;

	const SubmissionTierSpec SPEC;

private:
	int m_sentTier = 0;
};
