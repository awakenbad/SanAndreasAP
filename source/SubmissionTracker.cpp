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
