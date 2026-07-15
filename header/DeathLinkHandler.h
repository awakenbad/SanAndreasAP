#pragma once
#include <CPlayerInfo.h>
#include <CWorld.h>

class DeathLinkHandler
{
public:
	void setEnabled(bool enabled);

	/// <summary>
	/// Keeps track of dying inside of the game to prevent sending infinite death checks
	/// </summary>
	/// <returns>if the player should send deathlink or not</returns>
	bool update();

	/// <summary>
	/// Kills the player in response to a Deathlink event from the server
	/// </summary>
	void killPlayer();

private:
	bool m_enabled = false;
	bool m_wasDead = false;
	bool m_suppressNextDeathBroadcast = false;
};
