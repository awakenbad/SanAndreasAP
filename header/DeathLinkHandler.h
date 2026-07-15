#pragma once
#include <CPlayerInfo.h>
#include <CWorld.h>

class DeathLinkHandler
{
public:
	void setEnabled(bool enabled);

	/// Call once per game tick. Returns true exactly once when a new, organic player death is
	/// detected - never for a death this handler itself triggered via killPlayer(), which
	/// would otherwise cause an infinite DeathLink echo between players.
	bool update();

	/// Kills the player artificially, in response to an incoming DeathLink from another player.
	void killPlayer();

private:
	bool m_enabled = false;
	bool m_wasDead = false;
	bool m_suppressNextDeathBroadcast = false;
};
