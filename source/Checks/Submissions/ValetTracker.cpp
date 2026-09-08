#include "ValetTracker.h"

#include <CStats.h>

namespace
{
	// Cars parked in total by the end of each level - the stat is cumulative across the whole
	// activity, and holds at your best run when a level is failed.
	constexpr int LEVEL_THRESHOLDS[] = { 3, 7, 12, 18, 25 };
}

ValetTracker::ValetTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, VALET_TIERS)
{
}

void ValetTracker::enforceSubmissionReward()
{
}

float ValetTracker::getProgress() const
{
	return CStats::GetStatValue(STAT_MOST_CARS_PARKED_ON_VALET_PARKING);
}

int ValetTracker::currentTier() const
{
	const int cars = static_cast<int>(getProgress());

	int tier = 0;
	for (int threshold : LEVEL_THRESHOLDS)
	{
		if (cars >= threshold) tier++;
	}
	return tier;
}
