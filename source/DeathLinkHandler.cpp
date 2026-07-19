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

	// Being busted refills health at the police station the same way dying does at the
	// hospital, so the respawn edge tracks both states.
	bool deadOrArrested = isDead || CWorld::Players[0].m_nPlayerState == PLAYERSTATE_HASBEENARRESTED;
	if (m_wasDeadOrArrested && !deadOrArrested)
	{
		m_respawnPending = true;
	}
	m_wasDeadOrArrested = deadOrArrested;

	if (justDied && m_suppressNextDeathBroadcast)
	{
		m_suppressNextDeathBroadcast = false;
		return false;
	}
	return justDied;
}

bool DeathLinkHandler::consumeRespawn()
{
	bool respawned = m_respawnPending;
	m_respawnPending = false;
	return respawned;
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
