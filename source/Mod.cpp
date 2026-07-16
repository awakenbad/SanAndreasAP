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
    processPendingDisplayMessages();

    if (m_checkGiver.getProgressiveMissionCounter() == 0 && !m_blockersSpawned)
    {
        spawnMissionBlockers();
    }
    else if (m_checkGiver.getProgressiveMissionCounter() > 0 && m_blockersSpawned)
    {
        removeMissionBlockers();
    }
	if (plugin::KeyPressed(VK_TAB))
	{
        m_weaponGiver.giveSprayCan();
	}
	parseIncomingMessages();
}

void Mod::spawnMissionBlockers()
{
    CStreaming::RequestModel(BLOCKER_MODEL_ID, 0);
    CStreaming::LoadAllRequestedModels(false);

    for (const auto& [missionId, pos] : missionStartPos) {
        CObject* blocker = CObject::Create(BLOCKER_MODEL_ID);

        if (blocker) {
            blocker->SetPosition(CVector(pos.x, pos.y, pos.z));
            blocker->SetIsStatic(true);
            blocker->bStreamingDontDelete = true;
            blocker->bDistanceFade = true;
            blocker->m_nObjectType = OBJECT_MISSION;
            CWorld::Add(blocker);
            m_missionBlockers[missionId] = blocker;
            m_checkGiver.giveMoney(1000);
        }
    }
    m_blockersSpawned = true;
}

void Mod::removeMissionBlockers()
{
    for (auto& [missionId, blocker] : m_missionBlockers) {
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
            m_lastTagSentTime = std::chrono::steady_clock::now();
            m_checkListener.confirmTagSent();
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

    auto now = std::chrono::steady_clock::now();
    if (now - m_lastTagSentTime < TAG_MESSAGE_DELAY)
    {
        m_pendingDisplayMessages.push({ now + TAG_MESSAGE_DELAY, text });
    }
    else
    {
        showHelpText(text);
    }
}

void Mod::processPendingDisplayMessages()
{
    auto now = std::chrono::steady_clock::now();
    while (!m_pendingDisplayMessages.empty() && m_pendingDisplayMessages.front().first <= now)
    {
        showHelpText(m_pendingDisplayMessages.front().second);
        m_pendingDisplayMessages.pop();
    }
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
