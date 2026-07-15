#include "DeathLinkHandler.h"
#include "common.h"
#include <CVehicle.h>

void DeathLinkHandler::setEnabled(bool enabled)
{
	m_enabled = enabled;
}

bool DeathLinkHandler::update()
{
	if (m_enabled)
	{
		CWorld::Players[0].m_bGetOutOfHospitalFree = true;
	}

	bool isDead = CWorld::Players[0].m_nPlayerState == PLAYERSTATE_HASDIED;
	bool justDied = isDead && !m_wasDead;
	m_wasDead = isDead;

	if (justDied && m_suppressNextDeathBroadcast)
	{
		m_suppressNextDeathBroadcast = false;
		return false;
	}
	return justDied;
}

void DeathLinkHandler::killPlayer()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return;

	m_suppressNextDeathBroadcast = true;

	player->m_fHealth = 0.0f;

	if (player->bInVehicle && player->m_pVehicle)
	{
		player->m_pVehicle->m_fHealth = 0.0f;
		player->m_pVehicle->BlowUpCar(player, false);
	}
}
