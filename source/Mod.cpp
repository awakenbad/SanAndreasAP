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
		m_apSocket.sendToServer("CHECK:" + m_checkListener.getMissionID() + "\n");
		break;
	case CheckEvent::PickUp:
		//m_apSocket.sendToServer("CHECK:0\n");
		break;
	case CheckEvent::None:
		break;
	}
	

	parseIncomingMessages();
}

void Mod::parseIncomingMessages()
{
	std::string msg;
	while (m_apSocket.tryGetMessage(msg)) {
		if (msg.rfind("GIVE:money:", 0) == 0) {
			int amount = std::stoi(msg.substr(std::string("GIVE:money:").size()));
			m_checkGiver.giveMoney(amount);
		}
	}
}

void Mod::receiveCurrentCheckEvent()
{
	m_currentEvent = m_checkListener.update();
}
