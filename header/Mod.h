#pragma once
#include <vector>
#include <APSocket.h>
#include "EdgeTriggeredKey.h"
#include "ParseUtils.h"
#include "CheckListener.h"
#include "CheckGiver.h"
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
	std::vector<CObject*> m_missionBlockers;
	bool m_blockersSpawned = false;
	DeathLinkHandler m_deathLinkHandler;
	SaveDataManager m_saveDataManager;
	AutoSaveManager m_autoSaveManager;
	NotificationOverlay m_notificationOverlay;
	TagBlipManager m_tagBlipManager;
	AmmuNationShop m_ammuNationShop;
	TrapHandler m_trapHandler;
	ReceivedItemLog m_receivedItemLog;
	PendingChecks<int> m_pendingShopChecks;

	// Everything with state the vanilla save can't hold. Both the save and the restore path walk
	// this one list, so a subsystem can't be persisted in one direction only. Order matters just
	// once: CheckListener re-takes its detection baselines in load(), and nothing else depends on
	// that, so it goes first out of habit rather than necessity.
	// Declared after the subsystems it points at - members are constructed in declaration order.
	std::vector<PersistentState*> m_persistentSubsystems;

	bool m_firstInGameTickHandled = false;
	// Invisible object used to detect that the world was rebuilt (load or new game). Blips are
	// part of the save file and come back intact, so they cannot see a load - world objects are
	// destroyed by one, which makes an object the only sentinel a save can't preserve.
	CObject* m_worldSentinel = nullptr;
	bool detectWorldWipe();

	EdgeTriggeredKey m_tagBlipToggleKey{ VK_F8 };

	EdgeTriggeredKey m_tagDebugToggleKey{ VK_F7 };
	bool m_showTagDebug = false;

	// TEMPORARY dev tools for the Badlands verification pass - counter down/up to spawn and
	// despawn mission blockers on demand. Remove before release.
	EdgeTriggeredKey m_debugDecrementKey{ VK_F9 };
	EdgeTriggeredKey m_debugIncrementKey{ VK_F10 };

	// TEMPORARY: F6 sends every Los Santos check in one burst, for testing Badlands from a
	// post-Green-Sabre save without replaying LS. Remove before release.
	EdgeTriggeredKey m_debugSendAllLsKey{ VK_F6 };

	// TEMPORARY: F5 fakes an incoming DeathLink kill, for testing what happens when one lands at
	// an awkward moment (mid-cutscene, in a vehicle, during a mission). Remove before release.
	EdgeTriggeredKey m_debugDeathLinkKey{ VK_F5 };

	void debugSendAllLosSantosChecks();

	void parseIncomingMessages();

	// One tick, in order - see start() for why the sequence matters.
	void pollDeathLink();
	bool updateWorldState();
	void applyRespawnHealthTopUp();
	void updateGameplaySystems();
	void updateMissionBlockers();
	void updateDebugHotkeys();
	void spawnSprayCanPickup();
	void spawnMissionBlockers();
	void removeMissionBlockers();
	void sendChecksToAP(CheckEvent t_event);

	// Grants everything the log says this save is still owed. Re-grants after a save rollback are
	// summarised in one line instead of one notification each, and skip one-shot effects.
	void applyPendingItems();
	// Grants one item's effect. Returns false when the effect name isn't one this build knows.
	bool applyItemEffect(const std::string& t_effectName, const std::string& t_value, bool t_isNew);
	void applyControlMessage(const std::string& t_name, const std::string& t_value);
	void persistAndRestoreState(bool t_worldWiped);
};

