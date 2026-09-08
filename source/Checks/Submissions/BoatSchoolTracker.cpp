#include "BoatSchoolTracker.h"

#include "EntityIDs.h"
#include "ScriptGlobals.h"

BoatSchoolTracker::BoatSchoolTracker(int t_submissionID)
	: SchoolTracker(t_submissionID, BOAT_SCHOOL_TIERS, BOAT_SCHOOL_SCORE_GLOBALS)
{
}

TestMedal BoatSchoolTracker::medalForTest(int t_testIndex) const
{
	const int base = BOAT_SCHOOL_MEDAL_GLOBALS_BASE + t_testIndex * 3;

	if (ScriptGlobals::read(base + 2) == 1) return TestMedal::Gold;
	if (ScriptGlobals::read(base + 1) == 1) return TestMedal::Silver;
	if (ScriptGlobals::read(base) == 1) return TestMedal::Bronze;
	return TestMedal::None;
}
