#include "DrivingSchoolTracker.h"

#include <CStats.h>

#include "EntityIDs.h"

namespace
{
	constexpr float NEW_GAME_DRIVING_SKILL = 0.0f;
	constexpr float MAX_DRIVING_SKILL = 1000.0f;
}

DrivingSchoolTracker::DrivingSchoolTracker(int t_submissionID)
	: SchoolTracker(t_submissionID, DRIVING_SCHOOL_TIERS, DRIVING_SCHOOL_SCORE_GLOBALS)
{
}

void DrivingSchoolTracker::enforceSubmissionReward()
{
	CStats::SetStatValue(STAT_DRIVING_SKILL, checkReceived ? MAX_DRIVING_SKILL : NEW_GAME_DRIVING_SKILL);
}
