#include "Mod.h"

Mod::Mod()
{
	m_apSocket.connectToServer("127.0.0.1", 12345);
	
}

void Mod::start()
{
	receiveCurrentCheckEvent();
	switch (m_currentEvent)
	{
	case CheckEvent::Mission:
        m_checkGiver.removeProgressiveMission();
		m_apSocket.sendToServer("CHECK:MISSION:" + m_checkListener.getMissionID() + "\n");
		break;
	case CheckEvent::PickUp:
		m_apSocket.sendToServer("CHECK:PICKUP:0\n");
		break;
	case CheckEvent::None:
		break;
	}
    
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
        m_checkGiver.giveProgressiveMission();
	}
	parseIncomingMessages();
}

void Mod::spawnMissionBlockers()
{
    CStreaming::RequestModel(BLOCKER_MODEL_ID, 0);
    CStreaming::LoadAllRequestedModels(false);

    for (const auto& [missionId, pos] : missionStartPos) {
        CObject* blocker = CObject::Create(BLOCKER_MODEL_ID);
        CMessages::AddMessageJumpQ(blocker ? "Blocker created" : "Blocker creation FAILED", 2000, 0);

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

void Mod::parseIncomingMessages()
{
    std::string msg;
    while (m_apSocket.tryGetMessage(msg)) {
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
    }
}

void Mod::receiveCurrentCheckEvent()
{
	m_currentEvent = m_checkListener.update();
}
