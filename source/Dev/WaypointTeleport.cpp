#include "WaypointTeleport.h"

#ifdef DEBUG

#include <CRadar.h>
#include <CVector.h>

#include "EdgeTriggeredKey.h"
#include "PlayerControl.h"
#include "Teleport.h"

namespace
{
	EdgeTriggeredKey g_teleportKey{ VK_F6 };

	const tRadarTrace* findWaypoint()
	{
		for (unsigned int t = 0; t < MAX_RADAR_TRACES; ++t)
		{
			const tRadarTrace& trace = CRadar::ms_RadarTrace[t];
			if (trace.m_bInUse && trace.m_nRadarSprite == RADAR_SPRITE_WAYPOINT) return &trace;
		}
		return nullptr;
	}
}

void WaypointTeleport::update()
{
	if (!g_teleportKey.justPressed() || !PlayerControl::isInControl()) return;

	const tRadarTrace* waypoint = findWaypoint();
	if (!waypoint) return;

	Teleport::toGround(CVector(waypoint->m_vecPos.x, waypoint->m_vecPos.y, 0.0f), true);
}

#else

void WaypointTeleport::update()
{
}

#endif
