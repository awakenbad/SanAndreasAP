#include "CheckGiver.h"

void CheckGiver::giveMoney(int t_amount)
{
	CWorld::Players[0].m_nMoney += t_amount;
}

void CheckGiver::giveWeapon(std::string t_weaponType)
{
}

void CheckGiver::giveProgressiveMission()
{
	progressiveMissionCounter++;
}

void CheckGiver::removeProgressiveMission()
{
	progressiveMissionCounter--;
	if (progressiveMissionCounter < 0)
	{
		progressiveMissionCounter = 0;
	}
}

void CheckGiver::giveProgressiveMap()
{
}

int CheckGiver::getProgressiveMissionCounter()
{
	return progressiveMissionCounter;
}

void CheckGiver::setProgressiveMissionCounter(int t_value)
{
	progressiveMissionCounter = t_value;
}
