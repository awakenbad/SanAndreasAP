#pragma once
#include <vector>
#include <APSocket.h>
#include "EdgeTriggeredKey.h"
#include "ParseUtils.h"
#include "CheckListener.h"
#include "CheckGiver.h"
#include "BranchProgress.h"
#include "MissionLocateBlocked.h"
#include "BlockedMarkerTint.h"
#include "LegacyBlockerCleanup.h"
#include "CStats.h"
#include "EntityIDs.h"
#include "CRunningScript.h"
#include "CTheScripts.h"
#include "DeathLinkHandler.h"
#include "PersistentState.h"
#include "ReceivedItemLog.h"
#include "SaveDataManager.h"
#include "AutoSaveManager.h"
#include "NotificationOverlay.h"
#include "ScreenScale.h"
#include "CollectibleBlipsManager.h"
#include "BlipTarget.h"
#include "AmmuNationShop.h"
#include "ShopMenuText.h"
#include "StreetRaceUnlock.h"
#include "SubmissionResumeLevel.h"
#include "CityUnlock.h"
#include "FastTravel.h"
#include "BranchControllers.h"
#include "TrapHandler.h"
#include "GameStorageHook.h"
#include "WaypointTeleport.h"

class Mod
{
public:
	Mod();
	void start();
	void drawOverlay();
	void drawMenuOverlay();
	void drawMissionCountsOnMap();
	void drawCollectiblesOnMap();

	void updateMenuState();

private:
	const CVector SPRAYCAN_PICKUP_POS{ 2493.5f, -1671.0f, 13.3f };
	static constexpr unsigned int SPRAYCAN_PICKUP_AMMO = 5000;

	const CVector CAMERA_PICKUP_POS{ -2026.0f, 164.0f, 28.6f };
	static constexpr unsigned int CAMERA_PICKUP_AMMO = 5000;

	const CVector JETPACK_PICKUP_POS{ 2030.0f, 1007.7f, 10.6f };

	static constexpr float MISSION_BLIP_TOLERANCE_SQ = 25.0f;

	const char* MOD_VERSION = "0.8.3";

	CheckListener m_checkListener;
	CheckGiver m_checkGiver;
	BranchProgress m_branchProgress;
	APSocket m_apSocket;
	DeathLinkHandler m_deathLinkHandler;
	SaveDataManager m_saveDataManager;
	AutoSaveManager m_autoSaveManager;
	NotificationOverlay m_notificationOverlay;
	CollectibleBlipsManager m_blipManager;
	AmmuNationShop m_ammuNationShop;
	TrapHandler m_trapHandler;
	ReceivedItemLog m_receivedItemLog;
	PendingChecks<int> m_pendingShopChecks;

	std::vector<PersistentState*> m_persistentSubsystems;

	bool m_firstInGameTickHandled = false;

	bool m_streetRacesUnlocked = false;
	bool m_newGameRegrantPending = false;
	bool m_newGameRegrantClockStarted = false;
	unsigned int m_newGameRegrantControlStartMs = 0;
	static constexpr unsigned int NEW_GAME_REGRANT_DELAY_MS = 4000;

	EdgeTriggeredKey m_tagBlipToggleKey{ VK_F8 };

	std::vector<std::string> m_deferredLines;

	void parseIncomingMessages();
	void handleMessage(const std::string& t_rawLine);

	void pollDeathLink();
	bool updateWorldState(bool t_loadHooked);
	std::vector<BlipTarget> collectBlipTargets();
	void applyRespawnHealthTopUp();
	void updateGameplaySystems();
	void spawnCollectiblePickups();
	void spawnPickupOnce(const CVector& t_position, int t_modelId, unsigned int t_ammo);
	void sendChecksToAP(CheckEvent t_event);

	void drawVersionLabel();
	void drawMissionCounts();
	void drawMissionCountsImpl(bool t_menuMap);
	const char* branchAtBlip(const CVector& t_pos) const;

	void applyPendingItems();
	bool applyItemEffect(const std::string& t_effectName, const std::string& t_value, bool t_isNew);
	void applyControlMessage(const std::string& t_name, const std::string& t_value);
	void persistAndRestoreState(bool t_worldWiped, bool t_loadHooked);
	void resetForNewGame();
};
