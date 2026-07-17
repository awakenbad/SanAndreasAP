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
	bool worldWiped = m_tagBlipManager.update(m_checkListener.getClaimedTags());
 	persistAndRestoreState(worldWiped);
	receiveCurrentCheckEvent();
    sendChecksToAP();

    if (m_checkGiver.getProgressiveMissionCounter() == 0 && !m_blockersSpawned)
    {
        spawnMissionBlockers();
        m_notificationOverlay.show("Note: You are out of Progressive Missions. Missions will be blocked until you unlock more.");
    }
    else if (m_checkGiver.getProgressiveMissionCounter() > 0 && m_blockersSpawned)
    {
        removeMissionBlockers();
    }
	if (m_sprayCanKey.justPressed())
	{
        m_checkGiver.giveWeapon("Spray Can", true);
	}

    if (m_debugDecrementKey.justPressed())
    {
        m_checkGiver.removeProgressiveMission();
        m_notificationOverlay.show("DEBUG: Progressive Mission -> " + std::to_string(m_checkGiver.getProgressiveMissionCounter()));
    }

    if (m_debugIncrementKey.justPressed())
    {
        m_checkGiver.giveProgressiveMission();
        m_notificationOverlay.show("DEBUG: Progressive Mission -> " + std::to_string(m_checkGiver.getProgressiveMissionCounter()));
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
    };

    auto it = messageByEffect.find(effectType);
    if (it == messageByEffect.end()) return;
    m_notificationOverlay.show(it->second.first, it->second.second);
}

void Mod::drawOverlay()
{
    m_notificationOverlay.draw();
    m_tagBlipManager.drawTagNumbers(m_checkListener.getClaimedTags());
}

void Mod::drawMenuOverlay()
{
    bool connected = m_apSocket.isConnected();

    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetScale(0.7f, 1.4f);
    CFont::SetColor(connected ? CRGBA(80, 220, 80, 255) : CRGBA(220, 80, 80, 255));
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetDropShadowPosition(1);
    CFont::SetBackground(false, false);
    CFont::SetWrapx(static_cast<float>(RsGlobal.maximumWidth));

    CFont::PrintString(20.0f, static_cast<float>(RsGlobal.maximumHeight) - 55.0f,
        connected ? "Archipelago: Connected" : "Archipelago: Disconnected");
}

void Mod::receiveCurrentCheckEvent()
{
	m_currentEvent = m_checkListener.update();
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
}
