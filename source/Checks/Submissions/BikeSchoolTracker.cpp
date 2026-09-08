#include "BikeSchoolTracker.h"

#include <CStats.h>

#include "EntityIDs.h"

namespace
{
	constexpr float NEW_GAME_BIKE_SKILL = 0.0f;
	constexpr float MAX_BIKE_SKILL = 1000.0f;
}

BikeSchoolTracker::BikeSchoolTracker(int t_submissionID)
	: SchoolTracker(t_submissionID, BIKE_SCHOOL_TIERS, BIKE_SCHOOL_SCORE_GLOBALS)
{
}

void BikeSchoolTracker::enforceSubmissionReward()
{
	CStats::SetStatValue(STAT_BIKE_SKILL, checkReceived ? MAX_BIKE_SKILL : NEW_GAME_BIKE_SKILL);
}
