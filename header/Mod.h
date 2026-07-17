#pragma once
#include <vector>
#include <APSocket.h>
#include "EdgeTriggeredKey.h"
#include "CheckListener.h"
#include "CheckGiver.h"
#include "CStats.h"
#include "EntityIDs.h"
#include "CRunningScript.h"
#include "CTheScripts.h"
#include "DeathLinkHandler.h"
#include "SaveDataManager.h"
#include "NotificationOverlay.h"
#include "TagBlipManager.h"
#include "AmmuNationShop.h"
#include "TrapHandler.h"

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

	// Call every frame from Events::drawMenuBackgroundEvent - shows the AP client connection
	// status in the menu corner so players see at a glance that everything works.
	void drawMenuOverlay();

private:
	const int BLOCKER_MODEL_ID = 2973;
	const int BARRICADE_MODEL_ID = MODEL_CJ_ROADBARRIER;
	const float BARRICADE_Z_OFFSET = 0.6f;

	// A permanent respawning spray can outside CJ's house, so tag hunting never requires
	// trips back for ammo.
	const CVector SPRAYCAN_PICKUP_POS{ 2493.5f, -1671.0f, 13.3f };
	static constexpr unsigned int SPRAYCAN_PICKUP_AMMO = 5000;

	CheckListener m_checkListener;
	CheckGiver m_checkGiver;
	APSocket m_apSocket;
	CheckEvent m_currentEvent;
	std::vector<CObject*> m_missionBlockers;
	bool m_blockersSpawned = false;
	DeathLinkHandler m_deathLinkHandler;
	SaveDataManager m_saveDataManager;
	NotificationOverlay m_notificationOverlay;
	TagBlipManager m_tagBlipManager;
	AmmuNationShop m_ammuNationShop;
	TrapHandler m_trapHandler;
	PendingChecks<int> m_pendingShopChecks;

	bool m_firstInGameTickHandled = false;
	bool m_showTagBlips = true;

	EdgeTriggeredKey m_tagBlipToggleKey{ VK_F8 };

	void parseIncomingMessages();
	void receiveCurrentCheckEvent();
	void spawnSprayCanPickup();
	void spawnMissionBlockers();
	void removeMissionBlockers();
	void sendChecksToAP();
	void showReceivedItemMessage(const std::string& effectType, const std::string& value);
	void persistAndRestoreState(bool t_worldWiped);
};

