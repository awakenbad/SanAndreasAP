#include "Mod.h"

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
	if (plugin::KeyPressed(VK_TAB))
	{
        m_weaponGiver.giveSprayCan();
	}

    bool debugDecrementKeyPressed = plugin::KeyPressed(VK_F9);
    if (debugDecrementKeyPressed && !m_debugDecrementKeyWasPressed)
    {
        m_checkGiver.removeProgressiveMission();
        m_notificationOverlay.show("DEBUG: Progressive Mission -> " + std::to_string(m_checkGiver.getProgressiveMissionCounter()));
    }
    m_debugDecrementKeyWasPressed = debugDecrementKeyPressed;

    bool debugIncrementKeyPressed = plugin::KeyPressed(VK_F10);
    if (debugIncrementKeyPressed && !m_debugIncrementKeyWasPressed)
    {
        m_checkGiver.giveProgressiveMission();
        m_notificationOverlay.show("DEBUG: Progressive Mission -> " + std::to_string(m_checkGiver.getProgressiveMissionCounter()));
    }
    m_debugIncrementKeyWasPressed = debugIncrementKeyPressed;

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
            if (m_checkListener.isStoryMission(std::stoi(missionIDStr)))
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
            showHelpText(msg.substr(7));
            continue;
        }

        if (msg.rfind("GIVE:", 0) != 0) continue;

        std::string rest = msg.substr(5); // strip "GIVE:"
        size_t colonPos = rest.find(':');

        std::string effectType = (colonPos == std::string::npos) ? rest : rest.substr(0, colonPos);
        std::string value = (colonPos == std::string::npos) ? "" : rest.substr(colonPos + 1);

        if (effectType == "money") {
            m_checkGiver.giveMoney(std::stoi(value));
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
            m_checkListener.submissionCheckWasReceived(122);
        }
        else if (effectType == "armor_upgrade")
        {
            m_checkListener.submissionCheckWasReceived(124);
        }
        else if (effectType == "fire_immunity")
        {
            m_checkListener.submissionCheckWasReceived(123);
        }
        else if (effectType == "stamina_upgrade")
        {
            m_checkListener.submissionCheckWasReceived(125);
        }
        else if (effectType == "taxi_nitro")
        {
            m_checkListener.submissionCheckWasReceived(121);
        }
        else if (effectType == "boxing_style")
        {
            m_checkListener.submissionCheckWasReceived(114);
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
    std::string text;
    if (effectType == "money") {
        text = "Archipelago: Received $" + value;
    }
    else if (effectType == "weapon") {
        text = "Archipelago: Received weapon (" + value + ")";
    }
    else if (effectType == "progressive_mission") {
        text = "Archipelago: Received a Progressive Mission";
    }
    else if (effectType == "progressive_map") {
        text = "Archipelago: Received a Progressive Map";
    }
    else if (effectType == "health_upgrade") {
        text = "Archipelago: Received Max Health Upgrade";
    }
    else if (effectType == "armor_upgrade") {
        text = "Archipelago: Received Max Armor Upgrade";
    }
    else if (effectType == "fire_immunity") {
        text = "Archipelago: Received Fire Immunity";
    }
    else if (effectType == "stamina_upgrade") {
        text = "Archipelago: Received Infinite Sprint";
    }
    else if (effectType == "taxi_nitro") {
        text = "Archipelago: Received Taxi Nitro";
    }
    else if (effectType == "boxing_style") {
        text = "Archipelago: Received Boxing Style";
    }
    else {
        return;
    }

    NotificationIcon icon = NotificationIcon::None;
    if (effectType == "money") icon = NotificationIcon::Money;
    else if (effectType == "progressive_mission") icon = NotificationIcon::ProgressiveMission;
    else if (effectType == "health_upgrade") icon = NotificationIcon::HealthUpgrade;
    else if (effectType == "armor_upgrade") icon = NotificationIcon::ArmorUpgrade;
    else if (effectType == "taxi_nitro") icon = NotificationIcon::Taxi;
    else if (effectType == "stamina_upgrade") icon = NotificationIcon::Stamina;
    else if (effectType == "fire_immunity") icon = NotificationIcon::FireImmunity;
    else if (effectType == "boxing_style") icon = NotificationIcon::Boxing;
    else if (effectType == "weapon") icon = NotificationIcon::Weapon;

    m_notificationOverlay.show(text, icon);
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

void Mod::showHelpText(const std::string& text)
{
    strncpy_s(m_helpMessageBuffer, sizeof(m_helpMessageBuffer), text.c_str(), _TRUNCATE);
    CHud::SetHelpMessage(m_helpMessageBuffer, true, false, false);
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
		m_checkGiver.setProgressiveMissionCounter(std::stoi(m_saveDataManager.getValue("progressive_mission", "1")));

		for (SubmissionTracker* tracker : m_checkListener.getSubmissionTrackers())
		{
			std::string prefix = "submission_" + std::to_string(tracker->getSubmissionID()) + "_";
			bool received = m_saveDataManager.getValue(prefix + "received", "0") == "1";
			bool completed = m_saveDataManager.getValue(prefix + "completed", "0") == "1";
			tracker->restoreState(received, completed);
		}
	}

	m_saveDataManager.setValue("progressive_mission", std::to_string(m_checkGiver.getProgressiveMissionCounter()));

	for (SubmissionTracker* tracker : m_checkListener.getSubmissionTrackers())
	{
		std::string prefix = "submission_" + std::to_string(tracker->getSubmissionID()) + "_";
		m_saveDataManager.setValue(prefix + "received", tracker->getCheckReceived() ? "1" : "0");
		m_saveDataManager.setValue(prefix + "completed", tracker->getSubmissionCompleted() ? "1" : "0");
	}
}
