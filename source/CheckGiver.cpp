#include "CheckGiver.h"
#include "common.h"
#include "CStreaming.h"
#include "WeaponData.h"

void CheckGiver::giveMoney(int t_amount)
{
	CWorld::Players[0].m_nMoney += t_amount;
}

void CheckGiver::giveWeapon(const std::string& t_weaponType, bool t_equip)
{
	auto it = weaponDataByName.find(t_weaponType);
	if (it == weaponDataByName.end()) return;

	CPlayerPed* player = FindPlayerPed();
	if (!player) return;

	const WeaponInfo& info = it->second;
	CStreaming::RequestModel(info.model, 2);
	CStreaming::LoadAllRequestedModels(false);
	player->GiveWeapon(info.type, info.ammo, true);
	if (t_equip)
	{
		player->SetCurrentWeapon(info.type);
	}
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

void CheckGiver::giveArmorRefill()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return;

	player->m_fArmour = static_cast<float>(CWorld::Players[0].m_nMaxArmour);
}

void CheckGiver::giveCarRepair()
{
	m_carRepairPending = true;
}

void CheckGiver::update()
{
	if (!m_carRepairPending) return;

	CPlayerPed* player = FindPlayerPed();
	if (!player || !player->bInVehicle || !player->m_pVehicle) return;

	m_carRepairPending = false;
	player->m_pVehicle->Fix();
	player->m_pVehicle->m_fHealth = 1000.0f;
}

int CheckGiver::getProgressiveMissionCounter()
{
	return progressiveMissionCounter;
}

void CheckGiver::setProgressiveMissionCounter(int t_value)
{
	progressiveMissionCounter = t_value;
}
