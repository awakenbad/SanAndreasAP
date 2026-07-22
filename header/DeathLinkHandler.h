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
	/// Kills the player in response to a Deathlink event from the server. Held until the player
	/// is actually in control - see PlayerControl - so a kill arriving mid-cutscene can't strand
	/// a scripted sequence. Returns true if the kill landed immediately.
	/// </summary>
	bool killPlayer();

	// True while a kill is waiting for the player to regain control.
	bool hasDeferredKill() const;

	// True exactly once after the player comes back from being wasted or busted. Used to top
	// health back up to the (possibly upgraded) max, since the game's own respawn refill
	// recomputes max health from its internal stat and ignores the tracker's override.
	bool consumeRespawn();

private:
	bool m_enabled = false;
	bool m_wasDead = false;
	bool m_wasDeadOrArrested = false;
	bool m_respawnPending = false;
	bool m_suppressNextDeathBroadcast = false;

	// A kill that arrived while the player wasn't in control. Only ever one: several DeathLinks
	// landing during one cutscene still only need CJ to die once.
	bool m_killDeferred = false;

	// Does the actual killing, with no control check.
	void applyKill();
};
