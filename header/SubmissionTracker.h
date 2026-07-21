#pragma once
#include <vector>

class SubmissionTracker
{
public:
	SubmissionTracker(int t_submissionID);
	virtual ~SubmissionTracker() = default;
	virtual void enforceSubmissionReward() = 0;
	void checkWasReceived();
	void submissionWasCompleted();
	int getSubmissionID();

	bool getCheckReceived() const;
	bool getSubmissionCompleted() const;
	void restoreState(bool t_checkReceived, bool t_submissionCompleted);

	// Tiered submissions append the slot index of each tier reached since the last poll (see
	// TieredSubmissionTracker). The rest complete in one go and add nothing.
	virtual void pollNewTierSlots(std::vector<int>& t_outSlots) {}

	// Highest tier already sent, persisted per submission. Meaningless for non-tiered ones.
	virtual int getSentTier() const { return 0; }
	virtual void restoreSentTier(int t_tier) {}
protected:
	const int SUBMISSION_ID;
	bool checkReceived = false;
	bool submissionCompleted = false;
};

