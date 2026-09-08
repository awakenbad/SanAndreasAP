#include "TaxiTracker.h"

#include <CCheat.h>
#include <eModelID.h>

namespace
{
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
		CCheat::m_aCheatsActive[CHEAT_ALL_TAXIS_NITRO] = false;
	}
}

float TaxiTracker::getProgress() const
{
	return static_cast<float>(*reinterpret_cast<int32_t*>(TAXI_FARES_ADDR));
}

bool TaxiTracker::isVehicleValid(int t_modelId) const
{
	return t_modelId == MODEL_TAXI
		|| t_modelId == MODEL_CABBIE;
}
