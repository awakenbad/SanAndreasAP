#pragma once
#include <APSocket.h>
#include "WeaponGiver.h"
#include "CheckListener.h"
#include "CheckGiver.h"
#include "CStats.h"
#include "EntityIDs.h"
#include "CRunningScript.h"
#include "CTheScripts.h"
#include "CHud.h"

class Mod
{
public:
	Mod();

	/// <summary>
	/// Starts mod logic
	/// </summary>
	void start();

private:
	const int BLOCKER_MODEL_ID = 2973;
	const float DISTANCE_TO_PLAYER = 60.0f;
	int lastValue = 0;
	bool initialized = false;

	WeaponGiver m_weaponGiver;
	CheckListener m_checkListener;
	CheckGiver m_checkGiver;
	APSocket m_apSocket;
	CheckEvent m_currentEvent;
	std::unordered_map<int, CObject*> m_missionBlockers;
	bool m_blockersSpawned = false;

	// CHud::SetHelpMessage appears to keep re-rendering from whatever pointer it's given
	// every frame rather than copying it immediately (every existing caller in this file
	// only ever passed string literals, which are safe for that reason). This buffer is a
	// Mod-lifetime-long, persistent backing store so the pointer we hand it never dangles.
	char m_helpMessageBuffer[400] = {};



	void parseIncomingMessages();
	void receiveCurrentCheckEvent();
	void spawnMissionBlockers();
	void removeMissionBlockers();
	void sendChecksToAP();
	void showReceivedItemMessage(const std::string& effectType, const std::string& value);
	void showHelpText(const std::string& text);
};

