#pragma once
#include <chrono>
#include <queue>
#include <APSocket.h>
#include "WeaponGiver.h"
#include "CheckListener.h"
#include "CheckGiver.h"
#include "CStats.h"
#include "EntityIDs.h"
#include "CRunningScript.h"
#include "CTheScripts.h"
#include "CHud.h"
#include "DeathLinkHandler.h"
#include "SaveDataManager.h"

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
	DeathLinkHandler m_deathLinkHandler;
	SaveDataManager m_saveDataManager;

	char m_helpMessageBuffer[400] = {};

	static constexpr std::chrono::seconds TAG_MESSAGE_DELAY{ 5 };
	std::chrono::steady_clock::time_point m_lastTagSentTime = std::chrono::steady_clock::now() - std::chrono::hours(24);
	std::queue<std::pair<std::chrono::steady_clock::time_point, std::string>> m_pendingDisplayMessages;

	void parseIncomingMessages();
	void receiveCurrentCheckEvent();
	void spawnMissionBlockers();
	void removeMissionBlockers();
	void sendChecksToAP();
	void showReceivedItemMessage(const std::string& effectType, const std::string& value);
	void showHelpText(const std::string& text);
	void processPendingDisplayMessages();
	void persistAndRestoreState();
};

