#include "Mod.h"
#include "CStreaming.h"
#include "CPools.h"
#include <unordered_map>

// std::stoi throws on malformed input, which would crash the whole game - socket messages and
// the companion save file are both external data that can't be trusted to be numeric.
static int parseIntOr(const std::string& text, int fallback)
{
	char* end = nullptr;
	long value = strtol(text.c_str(), &end, 10);
	if (end == text.c_str()) return fallback;
	return static_cast<int>(value);
}

Mod::Mod()
{
	m_apSocket.connectToServer("127.0.0.1", 12345);
	
}

void Mod::start()
{
	m_apSocket.update();
	if (m_deathLinkHandler.update())
	{
		m_apSocket.sendToServer("PLAYER_DIED\n");
	}
	m_tagBlipManager.setBlipsEnabled(m_showTagBlips);
	bool worldWiped = m_tagBlipManager.update(m_checkListener.getClaimedTags());
 	persistAndRestoreState(worldWiped);
	receiveCurrentCheckEvent();
    sendChecksToAP();
    m_ammuNationShop.update();
    m_trapHandler.update();
    m_checkGiver.update();

    int purchasedSlot = m_ammuNationShop.pollPurchasedSlot();
    if (purchasedSlot >= 0)
    {
        m_pendingShopChecks.push(purchasedSlot);
        m_notificationOverlay.show("Archipelago: Checked Ammu-Nation (" + std::string(shopItems[purchasedSlot].displayName) + ")");
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
	parseIncomingMessages();
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
    m_blockersSpawned = true;
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

void Mod::sendChecksToAP()
{
    switch (m_currentEvent)
    {
    case CheckEvent::Mission:
    {
        std::string missionIDStr = m_checkListener.getMissionID();
        if (m_apSocket.sendToServer("CHECK:MISSION:" + missionIDStr + "\n"))
        {
            if (m_checkListener.isStoryMission(parseIntOr(missionIDStr, -1)))
            {
                m_checkGiver.removeProgressiveMission();
            }
            m_checkListener.confirmMissionSent();
        }
        break;
    }
    case CheckEvent::PickUp:
        if (m_apSocket.sendToServer("CHECK:PICKUP:0\n"))
        {
            m_checkListener.confirmPickUpSent();
            m_notificationOverlay.show("Picked up an item");
        }
        break;
    case CheckEvent::Tag:
        if (m_apSocket.sendToServer("CHECK:TAG:" + std::to_string(m_checkListener.getPendingTagIndex()) + "\n"))
        {
            m_checkListener.confirmTagSent();
        }
        break;
    case CheckEvent::Submission:
        if (m_apSocket.sendToServer("CHECK:MISSION:" + std::to_string(m_checkListener.getPendingSubmissionId()) + "\n"))
        {
            m_checkListener.confirmSubmissionSent();
        }
        break;
    case CheckEvent::None:
        break;
    }

    // Shop purchases live outside CheckListener's event system - send independently.
    if (m_pendingShopChecks.hasPending())
    {
        if (m_apSocket.sendToServer("CHECK:SHOP:" + std::to_string(m_pendingShopChecks.front()) + "\n"))
        {
            m_pendingShopChecks.confirm();
        }
    }
}

void Mod::parseIncomingMessages()
{
    std::string msg;
    while (m_apSocket.tryGetMessage(msg)) {
        if (msg.rfind("STATUS:", 0) == 0)
        {
            m_notificationOverlay.show(msg.substr(7));
            continue;
        }

        // An item we found that belongs to another player's world.
        if (msg.rfind("SENT:", 0) == 0)
        {
            m_notificationOverlay.show(msg.substr(5), NotificationIcon::ItemSent);
            continue;
        }

        if (msg.rfind("LOCATE:TAG:", 0) == 0)
        {
            int tagIndex = parseIntOr(msg.substr(11), -1);
            m_tagBlipManager.setLocatedTag(tagIndex);
            if (tagIndex >= 0)
            {
                m_notificationOverlay.show("Locating LS Tag #" + std::to_string(tagIndex + 1));
            }
            continue;
        }

        if (msg.rfind("SHOPITEM:", 0) == 0)
        {
            std::string rest = msg.substr(9);
            size_t colon = rest.find(':');
            if (colon != std::string::npos)
            {
                m_ammuNationShop.setSlotContents(parseIntOr(rest.substr(0, colon), -1), rest.substr(colon + 1));
            }
            continue;
        }

        if (msg.rfind("GIVE:", 0) != 0) continue;

        std::string rest = msg.substr(5); // strip "GIVE:"
        size_t colonPos = rest.find(':');

        std::string effectType = (colonPos == std::string::npos) ? rest : rest.substr(0, colonPos);
        std::string value = (colonPos == std::string::npos) ? "" : rest.substr(colonPos + 1);

        if (effectType == "money") {
            m_checkGiver.giveMoney(parseIntOr(value, 0));
        }
        else if (effectType == "weapon") {
            m_checkGiver.giveWeapon(value);
        }
        else if (effectType == "progressive_mission") {
            m_checkGiver.giveProgressiveMission();
        }
        else if (effectType == "progressive_map") {
            m_checkGiver.giveProgressiveMap();
        }
        else if (effectType == "health_upgrade")
        {
            m_checkListener.submissionCheckWasReceived(PARAMEDIC_ID);
        }
        else if (effectType == "armor_upgrade")
        {
            m_checkListener.submissionCheckWasReceived(VIGILANTE_ID);
        }
        else if (effectType == "fire_immunity")
        {
            m_checkListener.submissionCheckWasReceived(FIREFIGHTER_ID);
        }
        else if (effectType == "stamina_upgrade")
        {
            m_checkListener.submissionCheckWasReceived(BURGLARY_ID);
        }
        else if (effectType == "taxi_nitro")
        {
            m_checkListener.submissionCheckWasReceived(TAXI_ID);
        }
        else if (effectType == "boxing_style")
        {
            m_checkListener.submissionCheckWasReceived(LOS_SANTOS_GYM_ID);
        }
        else if (effectType == "armor_refill")
        {
            m_checkGiver.giveArmorRefill();
        }
        else if (effectType == "car_repair")
        {
            m_checkGiver.giveCarRepair();
        }
        else if (effectType.rfind("trap_", 0) == 0)
        {
            m_trapHandler.giveTrap(effectType.substr(5));
        }
        else if (effectType == "death_link")
        {
            m_deathLinkHandler.setEnabled(value == "1");
        }
        else if (effectType == "deathlink_kill")
        {
            m_deathLinkHandler.killPlayer();
        }
        else
        {
            continue;
        }

        showReceivedItemMessage(effectType, value);
    }
}

void Mod::showReceivedItemMessage(const std::string& effectType, const std::string& value)
{
    if (effectType == "money") {
        m_notificationOverlay.show("Archipelago: Received $" + value, NotificationIcon::Money);
        return;
    }
    if (effectType == "weapon") {
        m_notificationOverlay.show("Archipelago: Received weapon (" + value + ")", NotificationIcon::Weapon);
        return;
    }

    static const std::unordered_map<std::string, std::pair<const char*, NotificationIcon>> messageByEffect = {
        { "progressive_mission", { "Archipelago: Received a Progressive Mission", NotificationIcon::ProgressiveMission } },
        { "progressive_map",     { "Archipelago: Received a Progressive Map",     NotificationIcon::None } },
        { "health_upgrade",      { "Archipelago: Received Max Health Upgrade",    NotificationIcon::HealthUpgrade } },
        { "armor_upgrade",       { "Archipelago: Received Max Armor Upgrade",     NotificationIcon::ArmorUpgrade } },
        { "fire_immunity",       { "Archipelago: Received Fire Immunity",         NotificationIcon::FireImmunity } },
        { "stamina_upgrade",     { "Archipelago: Received Infinite Sprint",       NotificationIcon::Stamina } },
        { "taxi_nitro",          { "Archipelago: Received Taxi Nitro",            NotificationIcon::Taxi } },
        { "boxing_style",        { "Archipelago: Received Boxing Style",          NotificationIcon::Boxing } },
        { "armor_refill",        { "Archipelago: Received Full Armor",            NotificationIcon::ArmorUpgrade } },
        { "car_repair",          { "Archipelago: Received Car Repair",            NotificationIcon::Taxi } },
        { "trap_tires",          { "Archipelago: Flat Tires Trap!",               NotificationIcon::Trap } },
        { "trap_fat",            { "Archipelago: Fat CJ Trap!",                   NotificationIcon::Trap } },
        { "trap_wanted",         { "Archipelago: Wanted Level Trap!",             NotificationIcon::Trap } },
        { "trap_carfire",        { "Archipelago: Car Fire Trap!",                 NotificationIcon::Trap } },
    };

    auto it = messageByEffect.find(effectType);
    if (it == messageByEffect.end()) return;
    m_notificationOverlay.show(it->second.first, it->second.second);
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
        m_showTagBlips = !m_showTagBlips;
    }

    bool connected = m_apSocket.isConnected();
    float bottom = static_cast<float>(RsGlobal.maximumHeight);

    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetScale(0.7f, 1.4f);
    CFont::SetColor(connected ? CRGBA(80, 220, 80, 255) : CRGBA(220, 80, 80, 255));
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetDropShadowPosition(1);
    CFont::SetBackground(false, false);
    CFont::SetWrapx(static_cast<float>(RsGlobal.maximumWidth));

    CFont::PrintString(20.0f, bottom - 100.0f,
        connected ? "Archipelago: Connected" : "Archipelago: Disconnected");

    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetScale(0.55f, 1.1f);
    CFont::SetColor(CRGBA(255, 255, 255, 255));
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetDropShadowPosition(1);
    CFont::SetBackground(false, false);

    CFont::PrintString(20.0f, bottom - 55.0f,
        m_showTagBlips ? "F8 - Tag blips on map: ON" : "F8 - Tag blips on map: OFF");
}

void Mod::receiveCurrentCheckEvent()
{
	m_currentEvent = m_checkListener.update();
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
	}

	if (restoreNeeded)
	{
		m_notificationOverlay.show("Archipelago: Restored progress (" + m_saveDataManager.getCurrentSaveKey() + ")");

		// The load that triggered this restore destroyed every world object, including any
		// spawned blockers - those pointers are dangling now and must be dropped without
		// CWorld::Remove/delete (calling either on them corrupts the world lists and crashes
		// later). Resetting m_blockersSpawned lets them respawn if the counter calls for it.
		m_missionBlockers.clear();
		m_blockersSpawned = false;
		m_checkListener.resyncBaselines();
		m_checkGiver.setProgressiveMissionCounter(parseIntOr(m_saveDataManager.getValue("progressive_mission", "1"), 1));

		for (const auto& tracker : m_checkListener.getSubmissionTrackers())
		{
			std::string prefix = "submission_" + std::to_string(tracker->getSubmissionID()) + "_";
			bool received = m_saveDataManager.getValue(prefix + "received", "0") == "1";
			bool completed = m_saveDataManager.getValue(prefix + "completed", "0") == "1";
			tracker->restoreState(received, completed);
		}

		std::string tagBits = m_saveDataManager.getValue("tags_claimed", std::string(100, '0'));
		std::array<bool, 100> claimed{};
		for (size_t i = 0; i < claimed.size(); ++i)
		{
			claimed[i] = i < tagBits.size() && tagBits[i] == '1';
		}
		m_checkListener.restoreClaimedTags(claimed);

		m_showTagBlips = m_saveDataManager.getValue("show_tag_blips", "1") == "1";
	}

	m_saveDataManager.setValue("progressive_mission", std::to_string(m_checkGiver.getProgressiveMissionCounter()));

	for (const auto& tracker : m_checkListener.getSubmissionTrackers())
	{
		std::string prefix = "submission_" + std::to_string(tracker->getSubmissionID()) + "_";
		m_saveDataManager.setValue(prefix + "received", tracker->getCheckReceived() ? "1" : "0");
		m_saveDataManager.setValue(prefix + "completed", tracker->getSubmissionCompleted() ? "1" : "0");
	}

	const std::array<bool, 100>& claimed = m_checkListener.getClaimedTags();
	std::string tagBits(claimed.size(), '0');
	for (size_t i = 0; i < claimed.size(); ++i)
	{
		if (claimed[i]) tagBits[i] = '1';
	}
	m_saveDataManager.setValue("tags_claimed", tagBits);
	m_saveDataManager.setValue("show_tag_blips", m_showTagBlips ? "1" : "0");
}
