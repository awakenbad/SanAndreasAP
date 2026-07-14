#pragma once
class SubmissionTracker
{
public:
	SubmissionTracker(int t_submissionID);
	virtual void enforceSubmissionReward() = 0;
	void checkWasReceived();
	void submissionWasCompleted();
	int getSubmissionID();
protected:
	const int SUBMISSION_ID;
	bool checkReceived = false;
	bool submissionCompleted = false;
};

