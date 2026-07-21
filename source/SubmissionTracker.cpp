#include "SubmissionTracker.h"
#include "SaveDataManager.h"
#include "ParseUtils.h"

SubmissionTracker::SubmissionTracker(int t_submissionID)
	: SUBMISSION_ID(t_submissionID)
{
}

void SubmissionTracker::checkWasReceived()
{
	checkReceived = true;
}

void SubmissionTracker::submissionWasCompleted()
{
	submissionCompleted = true;
}

int SubmissionTracker::getSubmissionID()
{
	return SUBMISSION_ID;
}

bool SubmissionTracker::getSubmissionCompleted() const
{
	return submissionCompleted;
}

void SubmissionTracker::restoreState(bool t_checkReceived, bool t_submissionCompleted)
{
	checkReceived = t_checkReceived;
	submissionCompleted = t_submissionCompleted;
}

std::string SubmissionTracker::keyPrefix() const
{
	return "submission_" + std::to_string(SUBMISSION_ID) + "_";
}

void SubmissionTracker::save(SaveDataManager& t_saveData)
{
	std::string prefix = keyPrefix();
	t_saveData.setValue(prefix + "received", checkReceived ? "1" : "0");
	t_saveData.setValue(prefix + "completed", submissionCompleted ? "1" : "0");
	t_saveData.setValue(prefix + "tier", std::to_string(getSentTier()));
}

void SubmissionTracker::load(const SaveDataManager& t_saveData)
{
	std::string prefix = keyPrefix();
	restoreState(t_saveData.getValue(prefix + "received", "0") == "1",
		t_saveData.getValue(prefix + "completed", "0") == "1");
	restoreSentTier(parseIntOr(t_saveData.getValue(prefix + "tier", "0"), 0));
}
