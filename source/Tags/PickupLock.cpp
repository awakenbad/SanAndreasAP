#include "PickupLock.h"

#include <CPickups.h>
#include <CTimer.h>

namespace
{
	constexpr unsigned int LOCK_REGENERATION_DELAY = 60000;
}

void PickupLock::setModelLocked(int t_modelIndex, bool t_locked)
{
	bool wantVisible = !t_locked;

	for (unsigned int i = 0; i < MAX_NUM_PICKUPS; ++i)
	{
		CPickup& pickup = CPickups::aPickUps[i];
		if (pickup.m_nPickupType == PICKUP_NONE) continue;
		if (pickup.m_nModelIndex != t_modelIndex) continue;

		if (t_locked)
		{
			pickup.m_nFlags.bDisabled = true;
			pickup.m_nRegenerationTime = CTimer::m_snTimeInMilliseconds + LOCK_REGENERATION_DELAY;
		}
		else if (pickup.m_nFlags.bDisabled)
		{
			pickup.m_nFlags.bDisabled = false;
			pickup.m_nRegenerationTime = 0;
		}

		if ((pickup.m_nFlags.bVisible != 0) == wantVisible) continue;

		pickup.m_nFlags.bVisible = wantVisible;
		if (t_locked) pickup.GetRidOfObjects();
	}
}
