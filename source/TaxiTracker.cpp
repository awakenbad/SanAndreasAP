#include "TaxiTracker.h"
#include <cstdint>

namespace
{
	// "Fares delivered", the same counter vanilla uses to decide the nitro reward. From the
	// GTAMods memory-address list, live-verified to track fares 1:1.
	constexpr uintptr_t TAXI_FARES_ADDR = 0xA49C30;
}

TaxiTracker::TaxiTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, TAXI_TIERS)
{
}

void TaxiTracker::enforceSubmissionReward()
{
	if (checkReceived)
	{
		CCheat::m_aCheatsActive[CHEAT_ALL_TAXIS_NITRO] = true;
	}
	else if (submissionCompleted)
	{
		// Unlike the set-once player fields the other trackers manage, vanilla re-asserts this
		// cheat flag continuously once the fare goal is reached, so suppressing the unearned
		// reward has to be held every tick - a one-shot revert gets overwritten again.
		CCheat::m_aCheatsActive[CHEAT_ALL_TAXIS_NITRO] = false;
	}
}

float TaxiTracker::getProgress() const
{
	return static_cast<float>(*reinterpret_cast<int32_t*>(TAXI_FARES_ADDR));
}
