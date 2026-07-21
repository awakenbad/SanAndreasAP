#pragma once
#include <vector>
#include <string>

class SaveDataManager;

class SubmissionTracker
{
public:
	SubmissionTracker(int t_submissionID);
	virtual ~SubmissionTracker() = default;
	virtual void enforceSubmissionReward() = 0;
	void checkWasReceived();
	void submissionWasCompleted();
	int getSubmissionID();

	bool getSubmissionCompleted() const;
	void restoreState(bool t_checkReceived, bool t_submissionCompleted);

	// Tiered submissions append the slot index of each tier reached since the last poll (see
	// TieredSubmissionTracker). The rest complete in one go and add nothing.
	virtual void pollNewTierSlots(std::vector<int>& t_outSlots) {}

	// Highest tier already sent, persisted per submission. Meaningless for non-tiered ones.
	virtual int getSentTier() const { return 0; }
	virtual void restoreSentTier(int t_tier) {}

	// Not PersistentState overrides: trackers are owned by CheckListener, which persists them as
	// part of its own state rather than registering each one with Mod separately. Non-virtual on
	// purpose - the parts that differ per subclass are already virtual (getSentTier /
	// restoreSentTier), so every tracker persists through this one implementation.
	void save(SaveDataManager& t_saveData);
	void load(const SaveDataManager& t_saveData);
protected:
	// "submission_<id>_" - each tracker's keys are namespaced by its own AP location ID.
	std::string keyPrefix() const;

	const int SUBMISSION_ID;
	bool checkReceived = false;
	bool submissionCompleted = false;
};

