#include "Mod.h"
#include "APProtocol.h"
#include "ItemEffects.h"
#include "CStreaming.h"
#include "CPools.h"
#include <CRadar.h>

Mod::Mod()
{
	m_apSocket.connectToServer("127.0.0.1", 12345);

	m_persistentSubsystems = { &m_checkListener, &m_checkGiver, &m_tagBlipManager, &m_receivedItemLog, &m_trapHandler };
}

void Mod::start()
{
    // The order of these phases is load-bearing; each one's comment says why.
    m_apSocket.update();
    pollDeathLink();

    bool worldWiped = updateWorldState();
    persistAndRestoreState(worldWiped);

    // Detection has to run before the respawn top-up: it re-asserts the trackers' max-health
    // override for this tick, which the top-up then heals to.
    CheckEvent event = m_checkListener.update();
    applyRespawnHealthTopUp();

    sendChecksToAP(event);
    updateGameplaySystems();
    updateMissionBlockers();

    parseIncomingMessages();
}

void Mod::pollDeathLink()
{
    if (m_deathLinkHandler.update())
    {
        m_apSocket.sendToServer(APProtocol::playerDied());
    }
}

// Returns true when the world was rebuilt this tick (a load or a new game).
bool Mod::updateWorldState()
{
    // Both signals are polled every tick (no short-circuit): the object sentinel catches loads,
    // while the blip pool still catches a New Game that clears the radar.
    // Wipe detection must run BEFORE the blip manager touches anything: after a load its handles
    // are stale, and acting on them would clear blips that now belong to the game.
    bool objectWiped = detectWorldWipe();
    if (objectWiped)
    {
        m_tagBlipManager.onWorldWiped();
    }
    bool blipWiped = m_tagBlipManager.update(m_checkListener.getClaimedTags());
    return blipWiped || objectWiped;
}

// The hospital/police respawn refill recomputes max health from the game's internal stat,
// ignoring the Paramedic tracker's override - so on the respawn edge, top current health up to
// our max. Heals to whatever the current max is, so it's a no-op without the upgrade and also
// corrects the reverse case (respawn granting MORE than an unearned max).
void Mod::applyRespawnHealthTopUp()
{
    if (!m_deathLinkHandler.consumeRespawn()) return;

    if (CPlayerPed* player = FindPlayerPed())
    {
        player->m_fHealth = static_cast<float>(CWorld::Players[0].m_nMaxHealth);
    }
}

void Mod::updateGameplaySystems()
{
    m_ammuNationShop.update();
    m_trapHandler.update();
    m_checkGiver.update();

    if (m_autoSaveManager.update())
    {
        m_notificationOverlay.showAboveRadar("Archipelago: Autosaved (slot 8)");
    }

    int purchasedSlot = m_ammuNationShop.pollPurchasedSlot();
    if (purchasedSlot >= 0)
    {
        m_pendingShopChecks.push(purchasedSlot);
        m_notificationOverlay.show("Archipelago: Checked Ammu-Nation (" + std::string(shopItems[purchasedSlot].displayName) + ")");
    }
}

void Mod::updateMissionBlockers()
{
    // Self-heal: if the objects behind our bookkeeping have been destroyed (a load we failed to
    // detect), drop the stale list so the check below respawns them. This deliberately does not
    // depend on the world-wipe signal - it verifies the objects themselves.
    if (m_blockersSpawned && !m_missionBlockers.empty()
        && !CPools::ms_pObjectPool->IsObjectValid(m_missionBlockers.front()))
    {
        m_missionBlockers.clear();
        m_blockersSpawned = false;
    }

    if (m_checkGiver.getProgressiveMissionCounter() == 0 && !m_blockersSpawned)
    {
        spawnMissionBlockers();
        m_notificationOverlay.show("Note: You are out of Progressive Missions. Missions will be blocked until you unlock more.");
    }
    else if (m_checkGiver.getProgressiveMissionCounter() > 0 && m_blockersSpawned)
    {
        removeMissionBlockers();
    }
}


bool Mod::detectWorldWipe()
{
	bool wiped = false;

	if (m_worldSentinel)
	{
		// The pool slot can be recycled by an unrelated object after ours is destroyed, so
		// confirm the model too - otherwise a reused slot would look like our sentinel.
		bool stillOurs = CPools::ms_pObjectPool->IsObjectValid(m_worldSentinel)
			&& m_worldSentinel->m_nModelIndex == BLOCKER_MODEL_ID;
		if (!stillOurs)
		{
			wiped = true;
			m_worldSentinel = nullptr;
		}
	}

	if (!m_worldSentinel)
	{
		CPlayerPed* player = FindPlayerPed();
		if (!player) return wiped;

		CStreaming::RequestModel(BLOCKER_MODEL_ID, 0);
		CStreaming::LoadAllRequestedModels(false);

		m_worldSentinel = CObject::Create(BLOCKER_MODEL_ID);
		if (m_worldSentinel)
		{
			// Parked far below the player so it can never be seen or collided with.
			CVector pos = player->GetPosition();
			m_worldSentinel->SetPosition(CVector(pos.x, pos.y, pos.z - 500.0f));
			m_worldSentinel->SetIsStatic(true);
			m_worldSentinel->bStreamingDontDelete = true;
			m_worldSentinel->bIsVisible = false;
			m_worldSentinel->bUsesCollision = false;
			m_worldSentinel->m_nObjectType = OBJECT_MISSION;
			CWorld::Add(m_worldSentinel);
		}
	}

	return wiped;
}

void Mod::spawnMissionBlockers()
{
    CStreaming::RequestModel(BLOCKER_MODEL_ID, 0);
    CStreaming::RequestModel(BARRICADE_MODEL_ID, 0);
    CStreaming::LoadAllRequestedModels(false);

    for (const Position& pos : missionStartPos) {
        CObject* blocker = CObject::Create(BLOCKER_MODEL_ID);

        if (blocker) {
            blocker->SetPosition(CVector(pos.x, pos.y, pos.z));
            blocker->SetIsStatic(true);
            blocker->bStreamingDontDelete = true;
            blocker->bDistanceFade = true;
            blocker->bIsVisible = false;
            blocker->m_nObjectType = OBJECT_MISSION;
            CWorld::Add(blocker);
            m_missionBlockers.push_back(blocker);
        }

        CObject* barricade = CObject::Create(BARRICADE_MODEL_ID);

        if (barricade) {
            barricade->SetPosition(CVector(pos.x, pos.y, pos.z + BARRICADE_Z_OFFSET));
            barricade->SetIsStatic(true);
            barricade->bStreamingDontDelete = true;
            barricade->bDistanceFade = true;
            barricade->m_nObjectType = OBJECT_MISSION;
            CWorld::Add(barricade);
            m_missionBlockers.push_back(barricade);
        }
    }
    // Only latch when objects actually appeared. Right after a game load the models may not be
    // streamed in yet and CObject::Create can return null for every position - latching then
    // would leave the player with no blockers and no retry, so try again next tick instead.
    m_blockersSpawned = !m_missionBlockers.empty();
}

void Mod::removeMissionBlockers()
{
    for (CObject* blocker : m_missionBlockers) {
        // Backstop against pointers that dangle because a game load destroyed the objects
        // without the load being detected. Note this can't catch a freed slot the new game
        // state has already reused - the restore-time reset above is the primary protection.
        if (!CPools::ms_pObjectPool->IsObjectValid(blocker)) continue;

        CWorld::Remove(blocker);
        delete blocker;
    }
    m_missionBlockers.clear();

    m_blockersSpawned = false;
}

void Mod::sendChecksToAP(CheckEvent t_event)
{
    switch (t_event)
    {
    case CheckEvent::Mission:
    {
        std::string missionIDStr = m_checkListener.getMissionID();
        if (m_apSocket.sendToServer(APProtocol::missionCheck(missionIDStr)))
        {
            if (m_checkListener.isStoryMission(parseIntOr(missionIDStr, -1)))
            {
                m_checkGiver.removeProgressiveMission();
            }
            m_checkListener.confirmMissionSent();
            // Arm an autosave now that the check is away and the counter has been spent, so
            // the save reflects the completed mission. It fires once the game is safe to save.
            m_autoSaveManager.requestSave();
        }
        break;
    }
    case CheckEvent::PickUp:
        if (m_apSocket.sendToServer(APProtocol::pickUpCheck()))
        {
            m_checkListener.confirmPickUpSent();
            m_notificationOverlay.show("Picked up an item");
        }
        break;
    case CheckEvent::Tag:
        if (m_apSocket.sendToServer(APProtocol::tagCheck(m_checkListener.getPendingTagIndex())))
        {
            m_checkListener.confirmTagSent();
        }
        break;
    case CheckEvent::Submission:
        if (m_apSocket.sendToServer(APProtocol::missionCheck(m_checkListener.getPendingSubmissionId())))
        {
            m_checkListener.confirmSubmissionSent();
            m_autoSaveManager.requestSave();
        }
        break;
    case CheckEvent::None:
        break;
    }

    // Per-level submission checks (Paramedic/Firefighter/Vigilante levels 1-12) live outside
    // CheckListener's event system - send independently.
    if (m_checkListener.hasPendingSubmissionLevel())
    {
        if (m_apSocket.sendToServer(APProtocol::submissionLevelCheck(m_checkListener.getPendingSubmissionLevelSlot())))
        {
            m_checkListener.confirmSubmissionLevelSent();
        }
    }

    // Shop purchases live outside CheckListener's event system - send independently.
    if (m_pendingShopChecks.hasPending())
    {
        if (m_apSocket.sendToServer(APProtocol::shopCheck(m_pendingShopChecks.front())))
        {
            m_pendingShopChecks.confirm();
        }
    }
}

void Mod::parseIncomingMessages()
{
    std::string rawLine;
    while (m_apSocket.tryGetMessage(rawLine))
    {
        APProtocol::Message message = APProtocol::parse(rawLine);

        switch (message.kind)
        {
        case APProtocol::MessageKind::Status:
            m_notificationOverlay.show(message.text);
            break;

        // An item we found that belongs to another player's world.
        case APProtocol::MessageKind::ItemSent:
            m_notificationOverlay.show(message.text, NotificationIcon::ItemSent);
            break;

        case APProtocol::MessageKind::LocateTag:
            m_tagBlipManager.setLocatedTag(message.index);
            if (message.index >= 0)
            {
                m_notificationOverlay.show("Locating LS Tag #" + std::to_string(message.index + 1));
            }
            break;

        case APProtocol::MessageKind::ShopItem:
            m_ammuNationShop.setSlotContents(message.index, message.text);
            break;

        // Buffered rather than applied here: the log decides what this save is actually owed,
        // and batching the whole tick's delivery lets a re-grant be summarised in one line.
        case APProtocol::MessageKind::Give:
            m_receivedItemLog.recordDelivered(message.index, message.effect, message.text);
            break;

        case APProtocol::MessageKind::Control:
            applyControlMessage(message.effect, message.text);
            break;

        case APProtocol::MessageKind::Unknown:
            break;
        }
    }

    applyPendingItems();
}

void Mod::applyControlMessage(const std::string& t_name, const std::string& t_value)
{
    // Never deduplicated and never announced - these are events, not items the player owns.
    if (t_name == "death_link")
    {
        m_deathLinkHandler.setEnabled(t_value == "1");
    }
    else if (t_name == "deathlink_kill")
    {
        m_deathLinkHandler.killPlayer();
    }
}

void Mod::applyPendingItems()
{
    // Nothing is applied until the save's mark has had its chance to load, which happens on the
    // first in-game tick. Applying while still in the menus would grant against a default mark
    // and then immediately roll back once the real save arrived.
    if (!m_firstInGameTickHandled) return;

    std::vector<ReceivedItem> pending = m_receivedItemLog.takePendingItems();
    if (pending.empty()) return;

    int restoredCount = 0;
    for (const ReceivedItem& item : pending)
    {
        if (applyItemEffect(item.effect, item.value, item.isNew) && !item.isNew)
        {
            restoredCount++;
        }
    }

    // A rollback can owe a save dozens of items at once; one line beats burying the screen.
    if (restoredCount > 0)
    {
        m_notificationOverlay.show("Archipelago: Restored " + std::to_string(restoredCount) + " items");
    }
}

bool Mod::applyItemEffect(const std::string& t_effectName, const std::string& t_value, bool t_isNew)
{
    const ItemEffectSpec* spec = findItemEffect(t_effectName);
    if (!spec) return false;

    // Traps are one-shot events, not possessions. Re-granting one to a rolled-back save would
    // punish the player a second time for an item they already suffered through.
    if (spec->effect == ItemEffect::Trap && !t_isNew) return true;

    switch (spec->effect)
    {
    case ItemEffect::Money:              m_checkGiver.giveMoney(parseIntOr(t_value, 0)); break;
    case ItemEffect::Weapon:             m_checkGiver.giveWeapon(t_value); break;
    case ItemEffect::ProgressiveMission: m_checkGiver.giveProgressiveMission(); break;
    case ItemEffect::ProgressiveMap:     m_checkGiver.giveProgressiveMap(); break;
    case ItemEffect::SubmissionCheck:    m_checkListener.submissionCheckWasReceived(spec->submissionId); break;
    case ItemEffect::ArmorRefill:        m_checkGiver.giveArmorRefill(); break;
    case ItemEffect::CarRepair:          m_checkGiver.giveCarRepair(); break;
    case ItemEffect::Trap:               m_trapHandler.giveTrap(spec->trapName); break;
    }

    // Re-grants are counted and summarised by the caller instead.
    if (t_isNew)
    {
        std::string message = formatItemMessage(*spec, t_value);
        if (!message.empty())
        {
            m_notificationOverlay.show(message, spec->icon);
        }
    }
    return true;
}


void Mod::drawOverlay()
{
    m_notificationOverlay.draw();
    m_tagBlipManager.drawTagNumbers(m_checkListener.getClaimedTags());
    m_ammuNationShop.drawShopContents();
    m_trapHandler.drawTimers();

}

void Mod::drawMenuOverlay()
{
    // The pause menu is also where the mod's little settings live - poll the toggle here,
    // since this only runs while a menu is open.
    if (m_tagBlipToggleKey.justPressed())
    {
        m_tagBlipManager.toggleBlips();
    }

    bool connected = m_apSocket.isConnected();
    float bottom = static_cast<float>(RsGlobal.maximumHeight);

    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetScale(ScreenScale::of(0.7f), ScreenScale::of(1.4f));
    CFont::SetColor(connected ? CRGBA(80, 220, 80, 255) : CRGBA(220, 80, 80, 255));
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetDropShadowPosition(1);
    CFont::SetBackground(false, false);
    CFont::SetWrapx(static_cast<float>(RsGlobal.maximumWidth));

    CFont::PrintString(ScreenScale::of(20.0f), bottom - ScreenScale::of(100.0f),
        connected ? "Archipelago: Connected" : "Archipelago: Disconnected");

    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetScale(ScreenScale::of(0.55f), ScreenScale::of(1.1f));
    CFont::SetColor(CRGBA(255, 255, 255, 255));
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetDropShadowPosition(1);
    CFont::SetBackground(false, false);

    CFont::PrintString(ScreenScale::of(20.0f), bottom - ScreenScale::of(55.0f),
        m_tagBlipManager.areBlipsEnabled() ? "F8 - Tag blips on map: ON" : "F8 - Tag blips on map: OFF");
}

void Mod::spawnSprayCanPickup()
{
	// Pickups created this way are stored in the game save's pickup pool, so spawning blindly
	// every session would stack duplicates - skip if ours (or the regeneration placeholder of
	// ours) is already in the pool.
	for (int i = 0; i < 620; ++i)
	{
		const CPickup& pickup = CPickups::aPickUps[i];
		if (pickup.m_nPickupType == PICKUP_NONE) continue;
		if (pickup.m_nModelIndex != MODEL_SPRAYCAN) continue;

		CVector pos = const_cast<CPickup&>(pickup).GetPosn();
		if (std::fabs(pos.x - SPRAYCAN_PICKUP_POS.x) < 2.0f && std::fabs(pos.y - SPRAYCAN_PICKUP_POS.y) < 2.0f)
		{
			return;
		}
	}

	CPickups::GenerateNewOne(SPRAYCAN_PICKUP_POS, MODEL_SPRAYCAN, PICKUP_ON_STREET, SPRAYCAN_PICKUP_AMMO, 0, false, nullptr);
}

void Mod::persistAndRestoreState(bool t_worldWiped)
{
	m_saveDataManager.poll();

	// The first-in-game-tick trigger covers a session whose menu ticks never ran (no sentinel
	// existed yet to observe the wipe); the wipe signal covers every load after that. Both are
	// guarded by the fresh-New-Game check: a brand new game has no last-passed mission at its
	// very first tick (any loadable save does; the intro's INITIAL mission passes long before
	// saving is even possible), and restoring another slot's data into a fresh game is the
	// failure mode that must stay impossible.
	bool firstInGameTick = !m_firstInGameTickHandled && FindPlayerPed();
	if (firstInGameTick)
	{
		m_firstInGameTickHandled = true;
	}

	bool restoreNeeded = false;
	if ((t_worldWiped || firstInGameTick) && CStats::LastMissionPassedName[0] != '\0')
	{
		restoreNeeded = m_saveDataManager.restoreFromCurrentLoadName();
	}

	// Any fresh world (session start, load, or new game) may be missing the spray can pickup.
	if (firstInGameTick || t_worldWiped)
	{
		spawnSprayCanPickup();

		// A wiped world destroyed every object we cached, blockers included - drop the dangling
		// pointers without CWorld::Remove/delete (the objects are already gone; touching them
		// corrupts the world lists) and let the counter check respawn them.
		//
		// This must key off the wipe itself, NOT off restoreNeeded: a wipe that doesn't trigger
		// an AP restore used to leave m_blockersSpawned stuck true with no objects behind it, so
		// the spawn branch could never fire again and the player kept playing unblocked.
		m_missionBlockers.clear();
		m_blockersSpawned = false;
	}

	if (restoreNeeded)
	{
		m_notificationOverlay.show("Archipelago: Restored progress (" + m_saveDataManager.getCurrentSaveKey() + ")");

		for (PersistentState* subsystem : m_persistentSubsystems)
		{
			subsystem->load(m_saveDataManager);
		}
	}

	// Staged every tick rather than only on change, so a save triggered by anything - the player,
	// an autosave, a mission end - always writes current values with no separate dirty tracking.
	for (PersistentState* subsystem : m_persistentSubsystems)
	{
		subsystem->save(m_saveDataManager);
	}
}
