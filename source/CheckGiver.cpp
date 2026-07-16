#include "CheckGiver.h"
#include "common.h"
#include "CStreaming.h"
#include "WeaponData.h"

void CheckGiver::giveMoney(int t_amount)
{
	CWorld::Players[0].m_nMoney += t_amount;
}

void CheckGiver::giveWeapon(std::string t_weaponType)
{
	auto it = weaponDataByName.find(t_weaponType);
	if (it == weaponDataByName.end()) return;

	CPlayerPed* player = FindPlayerPed();
	if (!player) return;

	const WeaponInfo& info = it->second;
	CStreaming::RequestModel(info.model, 2);
	CStreaming::LoadAllRequestedModels(false);
	player->GiveWeapon(info.type, info.ammo, true);
	CStreaming::SetModelIsDeletable(info.model);
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
