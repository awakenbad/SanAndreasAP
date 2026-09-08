#include "ExportListTracker.h"

#include <CStats.h>
#include <common.h>

#include "ExportVehicles.h"

ExportListTracker::ExportListTracker()
	: Collectible<30>({}, 0, "export_claimed", "EXPORT")
{
}

bool ExportListTracker::update()
{
	rememberDrivenVehicle();
	return Collectible<30>::update();
}

float ExportListTracker::readCount() const
{
	return CStats::GetStatValue(STAT_NUMBER_OF_VEHICLES_EXPORTED);
}

int ExportListTracker::identifyCollected() const
{
	// Already claimed means we cannot tell which vehicle this was - drop the check rather than
	// claim one twice, which would burn a location the player never delivered.
	if (m_lastDrivenIndex < 0 || isClaimed(m_lastDrivenIndex)) return -1;

	return m_lastDrivenIndex;
}

void ExportListTracker::rememberDrivenVehicle()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player || !player->bInVehicle || !player->m_pVehicle) return;

	const int model = player->m_pVehicle->m_nModelIndex;
	for (int i = 0; i < static_cast<int>(exportVehicleModels.size()); ++i)
	{
		if (exportVehicleModels[i] == model)
		{
			m_lastDrivenIndex = i;
			return;
		}
	}
}
