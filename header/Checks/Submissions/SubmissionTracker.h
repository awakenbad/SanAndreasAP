#pragma once
#include "Lockable.h"

#include <string>
#include <vector>

class SaveDataManager;

class SubmissionTracker : public Lockable
{
public:
	SubmissionTracker(int t_submissionID);
	virtual ~SubmissionTracker() = default;
	virtual void enforceSubmissionReward() = 0;
	void checkWasReceived();
	void submissionWasCompleted();
	int getSubmissionID();
	virtual bool isVehicleValid(int t_modelId) const { return false; };

	bool getSubmissionCompleted() const;
	void restoreState(bool t_checkReceived, bool t_submissionCompleted);

	virtual void pollNewTierSlots(std::vector<int>& t_outSlots) {}

	virtual bool pollCompletion() { return false; }

	virtual std::string getSentState() const { return ""; }
	virtual void restoreSentState(const std::string& t_state) {}

	void save(SaveDataManager& t_saveData);
	void load(const SaveDataManager& t_saveData);
protected:
	std::string keyPrefix() const;

	const int SUBMISSION_ID;
	bool checkReceived = false;
	bool submissionCompleted = false;
};

