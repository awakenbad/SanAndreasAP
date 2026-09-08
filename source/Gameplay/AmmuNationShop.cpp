#include "AmmuNationShop.h"
#include "common.h"
#include "RunningScripts.h"
#include "CMenuSystem.h"
#include "CStreaming.h"
#include "WeaponData.h"
#include <CFont.h>
#include <CRGBA.h>
#include <CWorld.h>
#include <CSprite2d.h>

namespace
{
	constexpr char SOLD_ROW_TEXT[] = "SOLD";
	constexpr char ARMOUR_GXT_KEY[] = "ARMOUR";
}

static int modelForWeaponType(int t_weaponType)
{
	for (const auto& [name, info] : weaponDataByName)
	{
		if (static_cast<int>(info.type) == t_weaponType) return static_cast<int>(info.model);
	}
	return -1;
}

bool AmmuNationShop::isShopScriptActive() const
{
	return RunningScripts::isActive("amunat");
}

void AmmuNationShop::snapshotPlayerState()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return;

	for (int i = 0; i < 13; ++i)
	{
		m_weaponSnapshot[i].type = static_cast<int>(player->m_aWeapons[i].m_eWeaponType);
		m_weaponSnapshot[i].clip = player->m_aWeapons[i].m_nAmmoInClip;
		m_weaponSnapshot[i].total = player->m_aWeapons[i].m_nAmmoTotal;
	}
	m_armourSnapshot = player->m_fArmour;
	m_moneySnapshot = CWorld::Players[0].m_nMoney;
	m_snapshotValid = true;
}

void AmmuNationShop::restorePlayerState(bool t_refundMoney)
{
	CPlayerPed* player = FindPlayerPed();
	if (!player || !m_snapshotValid) return;

	for (int i = 0; i < 13; ++i)
	{
		CWeapon& weapon = player->m_aWeapons[i];
		const WeaponSlotSnapshot& snap = m_weaponSnapshot[i];

		if (static_cast<int>(weapon.m_eWeaponType) == snap.type)
		{
			weapon.m_nAmmoInClip = snap.clip;
			weapon.m_nAmmoTotal = snap.total;
		}
		else if (snap.type == WEAPONTYPE_UNARMED)
		{
			player->ClearWeapon(weapon.m_eWeaponType);
		}
		else
		{
			player->ClearWeapon(weapon.m_eWeaponType);

			int model = modelForWeaponType(snap.type);
			if (model >= 0)
			{
				CStreaming::RequestModel(model, 2);
				CStreaming::LoadAllRequestedModels(false);
				player->GiveWeapon(static_cast<eWeaponType>(snap.type), snap.total, true);
				CStreaming::SetModelIsDeletable(model);
				player->m_aWeapons[i].m_nAmmoInClip = snap.clip;
				player->m_aWeapons[i].m_nAmmoTotal = snap.total;
			}
		}
	}
	player->m_fArmour = m_armourSnapshot;

	if (t_refundMoney)
	{
		CWorld::Players[0].m_nMoney = m_moneySnapshot;
		CWorld::Players[0].m_nDisplayMoney = m_moneySnapshot;
	}
}

void AmmuNationShop::update()
{
	if (!isShopScriptActive())
	{
		m_confirmPanelItem = -1;
		m_boughtVisible = false;
		m_snapshotValid = false;
		restoreMaxArmour();
		return;
	}

	lockSoldRows();

	for (int panel = 0; panel < 2; ++panel)
	{
		if (!CMenuSystem::MenuInUse[panel]) continue;
		tMenuPanel* menu = MenuNumber[panel];
		if (!menu) continue;
		if (menu->m_nNumColumns != 2) continue; // only the Weapon/Cost confirm panel

		const char* cellKey = menu->m_aaacRowTitles[0][0];

		if (_strnicmp(cellKey, "BOUGHT", 10) == 0)
		{
			if (!m_boughtVisible)
			{
				m_boughtVisible = true;
				if (m_confirmPanelItem >= 0 && m_slotSold[m_confirmPanelItem])
				{
					restorePlayerState(true);
				}
				else if (m_confirmPanelItem >= 0 && !m_slotContents[m_confirmPanelItem].empty())
				{
					restorePlayerState(false);
					m_pendingPurchasedSlot = m_confirmPanelItem;
					m_slotSold[m_confirmPanelItem] = true;
				}
			}
		}
		else
		{
			m_boughtVisible = false;
			m_confirmPanelItem = -1;
			for (int i = 0; i < static_cast<int>(shopItems.size()); ++i)
			{
				if (_strnicmp(cellKey, shopItems[i].gxtKey, 10) == 0)
				{
					m_confirmPanelItem = i;
					break;
				}
			}
			snapshotPlayerState();
		}
	}

	updateArmourCeiling();
}

void AmmuNationShop::updateArmourCeiling()
{
	bool armourCheckOnOffer = m_confirmPanelItem >= 0 && !m_boughtVisible
		&& !m_slotSold[m_confirmPanelItem] && !m_slotContents[m_confirmPanelItem].empty()
		&& _strnicmp(shopItems[m_confirmPanelItem].gxtKey, ARMOUR_GXT_KEY, 10) == 0;

	if (armourCheckOnOffer) allowArmourPurchase();
	else restoreMaxArmour();
}

int AmmuNationShop::pollPurchasedSlot()
{
	int slot = m_pendingPurchasedSlot;
	m_pendingPurchasedSlot = -1;
	return slot;
}

void AmmuNationShop::setSlotContents(int t_slot, const std::string& t_text)
{
	if (t_slot < 0 || t_slot >= static_cast<int>(m_slotContents.size())) return;

	std::string plainText;
	plainText.reserve(t_text.size());
	for (char character : t_text)
	{
		if (character != '~') plainText.push_back(character);
	}
	m_slotContents[t_slot] = plainText;
}

void AmmuNationShop::setSlotFlags(int t_slot, int t_flags)
{
	if (t_slot < 0 || t_slot >= static_cast<int>(m_slotFlags.size())) return;

	m_slotFlags[t_slot] = t_flags;
}

int AmmuNationShop::slotForKey(const char* t_gxtKey) const
{
	for (int i = 0; i < static_cast<int>(shopItems.size()); ++i)
	{
		if (_strnicmp(t_gxtKey, shopItems[i].gxtKey, 10) == 0) return i;
	}
	return -1;
}

void AmmuNationShop::setSlotSold(int t_slot, bool t_sold)
{
	if (t_slot < 0 || t_slot >= static_cast<int>(m_slotSold.size())) return;
	m_slotSold[t_slot] = t_sold;
}

const char* AmmuNationShop::shopItemName(const char* t_gxtKey) const
{
	int slot = slotForKey(t_gxtKey);
	if (slot < 0) return nullptr;

	if (m_slotSold[slot]) return SOLD_ROW_TEXT;
	if (m_slotContents[slot].empty()) return nullptr;

	return m_slotContents[slot].c_str();
}

int AmmuNationShop::shopItemFlags(const char* t_gxtKey) const
{
	int slot = slotForKey(t_gxtKey);
	if (slot < 0 || m_slotSold[slot] || m_slotContents[slot].empty()) return -1;

	return m_slotFlags[slot];
}

void AmmuNationShop::allowArmourPurchase()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return;

	CPlayerInfo& info = CWorld::Players[0];
	if (!m_maxArmourRaised)
	{
		if (player->m_fArmour < static_cast<float>(info.m_nMaxArmour)) return; // vanilla already allows it

		m_maxArmourOriginal = info.m_nMaxArmour;
		m_maxArmourRaised = true;
	}

	info.m_nMaxArmour = static_cast<unsigned char>(player->m_fArmour) + 1;
}

void AmmuNationShop::restoreMaxArmour()
{
	if (!m_maxArmourRaised) return;

	CWorld::Players[0].m_nMaxArmour = m_maxArmourOriginal;
	m_maxArmourRaised = false;
}

void AmmuNationShop::lockSoldRows() const
{
	for (int panel = 0; panel < 2; ++panel)
	{
		if (!CMenuSystem::MenuInUse[panel]) continue;

		tMenuPanel* menu = MenuNumber[panel];
		if (!menu || menu->m_nNumColumns != 1) continue;

		for (int row = 0; row < menu->m_nNumRows && row < 12; ++row)
		{
			int slot = slotForKey(menu->m_aaacRowTitles[0][row]);
			if (slot >= 0 && m_slotSold[slot]) menu->m_abRowSelectable[row] = false;
		}
	}
}
