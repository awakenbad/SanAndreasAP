#include "RaceTracker.h"

#include "ScriptGlobals.h"

namespace
{
	constexpr int PASSED = 1;
}

RaceTracker::RaceTracker(int t_submissionID, const SubmissionTierSpec& t_spec, int t_firstRaceIndex)
	: SubmissionTracker(t_submissionID), SPEC(t_spec), FIRST_RACE_INDEX(t_firstRaceIndex)
{
}

void RaceTracker::pollNewTierSlots(std::vector<int>& t_outSlots)
{
	for (int tier = 0; tier < SPEC.tierCount; ++tier)
	{
		if (m_sent[tier]) continue;

		int global = RACE_PASSED_GLOBALS_BASE + FIRST_RACE_INDEX + tier;
		if (ScriptGlobals::read(global) != PASSED) continue;

		m_sent[tier] = true;
		t_outSlots.push_back(SPEC.baseSlot + tier);
	}
}
