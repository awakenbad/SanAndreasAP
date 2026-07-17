#include "AmmuNationShop.h"
#include "common.h"
#include "CTheScripts.h"
#include "CMenuSystem.h"
#include "CStreaming.h"
#include "WeaponData.h"
#include <CFont.h>
#include <CRGBA.h>
#include <CSprite2d.h>

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
	for (CRunningScript* script = CTheScripts::pActiveScripts; script; script = script->m_pNext)
	{
		if (_strnicmp(script->m_szName, "amunat", 8) == 0) return true;
	}
	return false;
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
	m_snapshotValid = true;
}

void AmmuNationShop::restorePlayerState()
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
			// The purchase put a brand-new weapon into a previously empty slot - take it away.
			player->ClearWeapon(weapon.m_eWeaponType);
		}
		else
		{
			// The purchase REPLACED a different weapon in this slot (e.g. a Silenced Pistol
			// bought while holding a Pistol). Remove the new one and stream the old one back
			// in with its previous ammo.
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
}

void AmmuNationShop::update()
{
	if (!isShopScriptActive())
	{
		m_confirmPanelItem = -1;
		m_boughtVisible = false;
		m_snapshotValid = false;
		return;
	}

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
				// Only slots the client has scouted contents for are AP locations - anything
				// else (excluded stock, offline play, already-checked slots pushed as empty)
				// stays a plain vanilla purchase.
				if (m_confirmPanelItem >= 0 && !m_slotContents[m_confirmPanelItem].empty())
				{
					restorePlayerState();
					m_pendingPurchasedSlot = m_confirmPanelItem;
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
			// Pre-purchase baseline, refreshed every tick the confirm panel shows an un-bought
			// item, so the restore on the BOUGHT edge reverts exactly this purchase.
			snapshotPlayerState();
		}
	}
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
	m_slotContents[t_slot] = t_text;
}

int AmmuNationShop::slotForKey(const char* t_gxtKey) const
{
	for (int i = 0; i < static_cast<int>(shopItems.size()); ++i)
	{
		if (_strnicmp(t_gxtKey, shopItems[i].gxtKey, 10) == 0) return i;
	}
	return -1;
}

void AmmuNationShop::drawContentsLine(float t_x, float t_y, const std::string& t_text) const
{
	float scale = static_cast<float>(RsGlobal.maximumHeight) / 1080.0f;

	CFont::SetFontStyle(FONT_SUBTITLES);
	CFont::SetScale(0.5f * scale, 1.0f * scale);
	CFont::SetColor(CRGBA(180, 230, 180, 255));
	CFont::SetProportional(true);
	CFont::SetOrientation(ALIGN_LEFT);
	CFont::SetDropShadowPosition(1);
	CFont::SetBackground(false, false);
	CFont::SetWrapx(static_cast<float>(RsGlobal.maximumWidth));

	float textWidth = CFont::GetStringWidth(const_cast<char*>(t_text.c_str()), true);
	CRect box(t_x - 8.0f * scale, t_y - 2.0f * scale, t_x + textWidth + 8.0f * scale, t_y + 30.0f * scale);
	CSprite2d::DrawRect(box, CRGBA(0, 0, 0, 150));

	CFont::PrintString(t_x, t_y, t_text.c_str());
}

void AmmuNationShop::drawShopContents()
{
	if (!isShopScriptActive()) return;

	float scale = static_cast<float>(RsGlobal.maximumHeight) / 1080.0f;

	for (int panel = 0; panel < 2; ++panel)
	{
		if (!CMenuSystem::MenuInUse[panel]) continue;
		tMenuPanel* menu = MenuNumber[panel];
		if (!menu) continue;

		if (menu->m_nNumColumns == 2)
		{
			// Confirm panel: one line under it for the single item on offer. Slots without
			// scouted contents aren't AP locations and get no line - they're vanilla stock.
			if (m_confirmPanelItem >= 0 && !m_slotContents[m_confirmPanelItem].empty())
			{
				drawContentsLine(menu->m_vPosn.x, menu->m_vPosn.y + 130.0f * scale,
					"Contains: " + m_slotContents[m_confirmPanelItem]);
			}
			continue;
		}

		// Weapon list: a contents column to the right of the native menu, one line per row
		// whose GXT key is a shop item with scouted contents. Category menus have no matching
		// keys, so nothing is drawn over them. Offsets tuned against the menu layout at 1080p.
		for (int row = 0; row < menu->m_nNumRows && row < 12; ++row)
		{
			int slot = slotForKey(menu->m_aaacRowTitles[0][row]);
			if (slot < 0 || m_slotContents[slot].empty()) continue;

			drawContentsLine(menu->m_vPosn.x + 620.0f * scale,
				menu->m_vPosn.y + (58.0f + 30.0f * static_cast<float>(row)) * scale,
				m_slotContents[slot]);
		}
	}
}
