#pragma once
#include <vector>
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
#include "NotificationOverlay.h"

class Mod
{
public:
	Mod();

	/// <summary>
	/// Starts mod logic
	/// </summary>
	void start();

	// Call every frame from Events::drawHudEvent, after the native HUD draw.
	void drawOverlay();

private:
	const int BLOCKER_MODEL_ID = 2973;
	const int BARRICADE_MODEL_ID = MODEL_CJ_ROADBARRIER;
	const float BARRICADE_Z_OFFSET = 0.6f;
	const float DISTANCE_TO_PLAYER = 60.0f;
	int lastValue = 0;
	bool initialized = false;

	WeaponGiver m_weaponGiver;
	CheckListener m_checkListener;
	CheckGiver m_checkGiver;
	APSocket m_apSocket;
	CheckEvent m_currentEvent;
	std::vector<CObject*> m_missionBlockers;
	bool m_blockersSpawned = false;
	DeathLinkHandler m_deathLinkHandler;
	SaveDataManager m_saveDataManager;
	NotificationOverlay m_notificationOverlay;

	char m_helpMessageBuffer[400] = {};

	bool m_debugDecrementKeyWasPressed = false;
	bool m_debugIncrementKeyWasPressed = false;

	// Temporary debug readout - remove once the AMBULAN/etc. GXT-key question is settled. Shows
	// the live CStats::LastMissionPassedName value plus the three HIGHEST_..._MISSION_LEVEL stats
	// every frame, top-left, so this can be observed passively during normal play instead of
	// needing a dedicated from-scratch test run.
	void drawDebugStatsOverlay();

	void parseIncomingMessages();
	void receiveCurrentCheckEvent();
	void spawnMissionBlockers();
	void removeMissionBlockers();
	void sendChecksToAP();
	void showReceivedItemMessage(const std::string& effectType, const std::string& value);
	void showHelpText(const std::string& text);
	void persistAndRestoreState();
};

