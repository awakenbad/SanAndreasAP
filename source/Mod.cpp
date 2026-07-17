#include "Mod.h"
#include "CStreaming.h"
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
 	persistAndRestoreState();
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
    drawDebugStatsOverlay();
}

void Mod::drawDebugStatsOverlay()
{
    int32_t taxiStatsValue = *reinterpret_cast<int32_t*>(0xB79078);
    int32_t taxiRewardValue = *reinterpret_cast<int32_t*>(0xA49C30);

    // Live m_nFightingStyle readout for the LS Gym investigation: need to observe whether it
    // jumps to 5 (STYLE_BOXING) automatically the moment the gym challenge completes, or only
    // after manually equipping the style. -1 = no player ped.
    int fightingStyle = -1;
    CPlayerPed* debugPlayer = FindPlayerPed();
    if (debugPlayer) fightingStyle = static_cast<int>(debugPlayer->m_nFightingStyle);

    // Active script names, to identify the LS gym's script while standing inside it. m_szName
    // is char[8] with no guaranteed null terminator, so copy with an explicit length cap.
    std::string scriptNames;
    int scriptCount = 0;
    for (CRunningScript* s = CTheScripts::pActiveScripts; s && scriptCount < 12; s = s->m_pNext, ++scriptCount)
    {
        scriptNames += std::string(s->m_szName, strnlen(s->m_szName, 8)) + " ";
    }

    std::string text = "DEBUG LastMission: " + std::string(CStats::LastMissionPassedName) + "~n~"
        + "DEBUG Vigilante=" + std::to_string(static_cast<int>(CStats::GetStatValue(STAT_HIGHEST_VIGILANTE_MISSION_LEVEL)))
        + " Paramedic=" + std::to_string(static_cast<int>(CStats::GetStatValue(STAT_HIGHEST_PARAMEDIC_MISSION_LEVEL)))
        + " Firefighter=" + std::to_string(static_cast<int>(CStats::GetStatValue(STAT_HIGHEST_FIREFIGHTER_MISSION_LEVEL))) + "~n~"
        + "DEBUG TaxiStatsAddr(0xB79078)=" + std::to_string(taxiStatsValue)
        + " TaxiRewardAddr(0xA49C30)=" + std::to_string(taxiRewardValue) + "~n~"
        + "DEBUG FightStyle=" + std::to_string(fightingStyle) + " (4=standard 5=boxing)" + "~n~"
        + "DEBUG Scripts: " + scriptNames;

    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetScale(0.5f, 1.0f);
    CFont::SetColor(CRGBA(255, 255, 0, 255));
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetDropShadowPosition(1);
    CFont::SetBackground(false, false);
    CFont::SetWrapx(700.0f);

    CFont::PrintString(20.0f, 20.0f, text.c_str());
}

void Mod::receiveCurrentCheckEvent()
{
	m_currentEvent = m_checkListener.update();
}

void Mod::persistAndRestoreState()
{
	bool restoreNeeded = m_saveDataManager.poll();
	if (restoreNeeded)
	{
		m_checkListener.resyncBaselines();
		m_checkGiver.setProgressiveMissionCounter(parseIntOr(m_saveDataManager.getValue("progressive_mission", "1"), 1));

		for (const auto& tracker : m_checkListener.getSubmissionTrackers())
		{
			std::string prefix = "submission_" + std::to_string(tracker->getSubmissionID()) + "_";
			bool received = m_saveDataManager.getValue(prefix + "received", "0") == "1";
			bool completed = m_saveDataManager.getValue(prefix + "completed", "0") == "1";
			tracker->restoreState(received, completed);
		}
	}

	m_saveDataManager.setValue("progressive_mission", std::to_string(m_checkGiver.getProgressiveMissionCounter()));

	for (const auto& tracker : m_checkListener.getSubmissionTrackers())
	{
		std::string prefix = "submission_" + std::to_string(tracker->getSubmissionID()) + "_";
		m_saveDataManager.setValue(prefix + "received", tracker->getCheckReceived() ? "1" : "0");
		m_saveDataManager.setValue(prefix + "completed", tracker->getSubmissionCompleted() ? "1" : "0");
	}
}
