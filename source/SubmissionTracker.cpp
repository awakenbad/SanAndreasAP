#include "SubmissionTracker.h"

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

bool SubmissionTracker::getCheckReceived() const
{
	return checkReceived;
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
