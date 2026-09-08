#include "ShopMenuText.h"

#include <CFont.h>
#include <CMenuSystem.h>
#include <CRGBA.h>
#include <CText.h>
#include <Patch.h>
#include <extensions/Screen.h>

#include "AmmuNationShop.h"
#include "ModSettings.h"

namespace
{
	constexpr uintptr_t MENU_ROW_TITLE_CALL_SITE = 0x58143E;
	constexpr float NAME_COLUMN_PADDING = 40.0f;
	constexpr float MIN_NAME_COLUMN_WIDTH = 268.0f;

	const AmmuNationShop* g_shop = nullptr;

	constexpr int FLAG_PROGRESSION = 1;
	constexpr int FLAG_USEFUL = 2;
	constexpr int FLAG_TRAP = 4;

	using ModSettings::ItemColour;

	CRGBA colourForFlags(int t_flags)
	{
		if (t_flags & FLAG_PROGRESSION) return ModSettings::itemColour(ItemColour::Progression);
		if (t_flags & FLAG_USEFUL) return ModSettings::itemColour(ItemColour::Useful);
		if (t_flags & FLAG_TRAP) return ModSettings::itemColour(ItemColour::Trap);
		return ModSettings::itemColour(ItemColour::Filler);
	}

	unsigned char dimChannel(unsigned char t_channel)
	{
		return static_cast<unsigned char>(t_channel * ModSettings::unselectedDimPercent() / 100);
	}

	CRGBA dimmed(const CRGBA& t_colour)
	{
		return CRGBA(dimChannel(t_colour.r), dimChannel(t_colour.g), dimChannel(t_colour.b), t_colour.a);
	}

	tMenuPanel* panelHoldingKey(const char* t_key, int& t_outRow)
	{
		for (int panel = 0; panel < 2; ++panel)
		{
			tMenuPanel* menu = MenuNumber[panel];
			if (!menu) continue;

			const char* first = &menu->m_aaacRowTitles[0][0][0];
			if (t_key < first || t_key >= first + sizeof(menu->m_aaacRowTitles)) continue;

			constexpr size_t CELL = sizeof(menu->m_aaacRowTitles[0][0]);
			constexpr size_t ROWS = sizeof(menu->m_aaacRowTitles[0]) / CELL;
			t_outRow = static_cast<int>((t_key - first) / CELL % ROWS);
			return menu;
		}
		return nullptr;
	}


	char* __fastcall onMenuRowTitle(CText* t_text, void*, char* t_key)
	{
		if (g_shop)
		{
			int row = -1;
			tMenuPanel* menu = panelHoldingKey(t_key, row);
			bool selected = menu && row == menu->m_nSelectedRow;

			if (const char* replacement = g_shop->shopItemName(t_key))
			{
				if (menu && menu->m_nNumColumns == 2)
				{
					float width = CFont::GetStringWidth(replacement, true, false)
						+ SCREEN_MULTIPLIER(NAME_COLUMN_PADDING);
					float floorWidth = SCREEN_MULTIPLIER(MIN_NAME_COLUMN_WIDTH);
					menu->m_afColumnWidth[0] = width > floorWidth ? width : floorWidth;
				}

				int flags = g_shop->shopItemFlags(t_key);
				if (flags >= 0)
				{
					CRGBA colour = colourForFlags(flags);
					CFont::SetColor(selected ? colour : dimmed(colour));
				}
				return const_cast<char*>(replacement);
			}
		}
		return const_cast<char*>(t_text->Get(t_key));
	}
}

void ShopMenuText::install(const AmmuNationShop& t_shop)
{
	if (g_shop) return;

	g_shop = &t_shop;
	plugin::patch::RedirectCall(MENU_ROW_TITLE_CALL_SITE, &onMenuRowTitle);
}
