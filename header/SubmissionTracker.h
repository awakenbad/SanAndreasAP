#pragma once
class SubmissionTracker
{
public:
	SubmissionTracker(int t_submissionID);
	virtual void enforceSubmissionReward() = 0;
	void checkWasReceived();
	void submissionWasCompleted();
	int getSubmissionID();

	bool getCheckReceived() const;
	bool getSubmissionCompleted() const;
	void restoreState(bool t_checkReceived, bool t_submissionCompleted);
protected:
	const int SUBMISSION_ID;
	bool checkReceived = false;
	bool submissionCompleted = false;
};

