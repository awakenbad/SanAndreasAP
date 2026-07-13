#pragma once
#include <APSocket.h>
#include "WeaponGiver.h"
#include "CheckListener.h"
#include "CheckGiver.h"
#include "CStats.h"
#include "EntityIDs.h"
#include "CRunningScript.h"
#include "CTheScripts.h"

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

	WeaponGiver m_weaponGiver;
	CheckListener m_checkListener;
	CheckGiver m_checkGiver;
	APSocket m_apSocket;
	CheckEvent m_currentEvent;
	std::unordered_map<int, CObject*> m_missionBlockers;
	bool m_blockersSpawned = false;
	


	void parseIncomingMessages();
	void receiveCurrentCheckEvent();
	void spawnMissionBlockers();
	void removeMissionBlockers();
};

