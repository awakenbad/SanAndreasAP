#include "CheckGiver.h"

#include <CClothes.h>
#include <CStats.h>
#include <CStreaming.h>
#include <CWorld.h>
#include <common.h>
#include <map>

#include "PlayerControl.h"
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

void CheckGiver::giveProgressiveMap()
{
}

namespace
{
	const std::map<std::string, unsigned short> weaponSkillByName = {
		{ "Pistol",            STAT_PISTOL_SKILL },
		{ "Silenced Pistol",   STAT_SILENCED_PISTOL_SKILL },
		{ "Desert Eagle",      STAT_DESERT_EAGLE_SKILL },
		{ "Shotgun",           STAT_SHOTGUN_SKILL },
		{ "Sawn-off Shotgun",  STAT_SAWN_OFF_SHOTGUN_SKILL },
		{ "Combat Shotgun",    STAT_COMBAT_SHOTGUN_SKILL },
		{ "Machine Pistol",    STAT_MACHINE_PISTOL_SKILL },
		{ "SMG",               STAT_SMG_SKILL },
		{ "AK-47",             STAT_AK_47_SKILL },
		{ "M4",                STAT_M4_SKILL },
		{ "Rifle",             STAT_RIFLE_SKILL },
	};

	constexpr float MAX_SKILL = 1000.0f;
}

void CheckGiver::giveWeaponMastery(const std::string& t_weaponName)
{
	auto it = weaponSkillByName.find(t_weaponName);
	if (it == weaponSkillByName.end()) return;

	CStats::SetStatValue(it->second, MAX_SKILL);
}

void CheckGiver::giveMaxSkill(int t_statId)
{
	CStats::SetStatValue(static_cast<unsigned short>(t_statId), MAX_SKILL);

	if (t_statId != STAT_MUSCLE || !PlayerControl::isInControl()) return;

	if (CPlayerPed* player = FindPlayerPed())
	{
		CClothes::RebuildPlayer(player, false);
	}
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
