#include "CheckGiver.h"
#include "common.h"
#include "CStreaming.h"
#include "WeaponData.h"
#include "SaveDataManager.h"
#include "ParseUtils.h"

namespace
{
	constexpr char PROGRESSIVE_MISSION_KEY[] = "progressive_mission";
}

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

void CheckGiver::save(SaveDataManager& t_saveData)
{
	t_saveData.setValue(PROGRESSIVE_MISSION_KEY, std::to_string(progressiveMissionCounter));
}

void CheckGiver::load(const SaveDataManager& t_saveData)
{
	// Defaults to 1, not 0: that is the single mission the player starts the game able to do, so
	// a save written before this key existed stays playable rather than instantly blocked.
	progressiveMissionCounter = parseIntOr(t_saveData.getValue(PROGRESSIVE_MISSION_KEY, "1"), 1);
}
