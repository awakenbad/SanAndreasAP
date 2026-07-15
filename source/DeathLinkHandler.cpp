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
		// Matches the vanilla "dating Katie Zhan" perk exactly: no money or weapon loss on
		// death, only armor. Enforced every tick so it survives respawn without re-toggling.
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

	// On foot, zeroing ped health triggers the death sequence directly.
	player->m_fHealth = 0.0f;

	// In a vehicle, zeroing ped health alone does NOT reliably kill the player (a
	// well-documented SA quirk - there's a dedicated "Death in Car" script for exactly this),
	// so blow up the car too, which kills the occupant.
	if (player->bInVehicle && player->m_pVehicle)
	{
		player->m_pVehicle->m_fHealth = 0.0f;
		player->m_pVehicle->BlowUpCar(player, false);
	}
}
