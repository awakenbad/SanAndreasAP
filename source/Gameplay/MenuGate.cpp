#include "MenuGate.h"

#include <CMenuManager.h>
#include <windows.h>

#include "EdgeTriggeredKey.h"

namespace
{
	constexpr int NEW_GAME_ENTRY = 0;
	constexpr int LOAD_GAME_ENTRY = 1;

	constexpr uintptr_t REFRESH_SAVE_SLOT_LIST = 0x619140;

	constexpr unsigned int EXPLANATION_MS = 5000;

	unsigned char g_openAction[2] = {};
	bool g_captured = false;
	bool g_applied = false;
	bool g_open = true;

	EdgeTriggeredKey g_confirmKey{ VK_RETURN };
	EdgeTriggeredKey g_clickKey{ VK_LBUTTON };
	bool g_confirmPending = false;
	unsigned int g_explainUntilMs = 0;

	void watchForBlockedConfirm()
	{
		bool onGatePage = FrontEndMenuManager.m_nCurrentMenuPage == MENUPAGE_NEW_GAME;

		if (g_confirmPending)
		{
			g_confirmPending = false;
			if (onGatePage) g_explainUntilMs = GetTickCount() + EXPLANATION_MS;
		}

		if (onGatePage && (g_confirmKey.justPressed() || g_clickKey.justPressed()))
		{
			g_confirmPending = true;
		}
	}
}

void MenuGate::update(bool t_worldKnown)
{
	CMenuScreen::CMenuEntry* entries = aScreens[MENUPAGE_NEW_GAME].m_aEntries;

	if (!g_captured)
	{
		g_openAction[0] = entries[NEW_GAME_ENTRY].m_nAction;
		g_openAction[1] = entries[LOAD_GAME_ENTRY].m_nAction;
		g_captured = true;
	}

	if (t_worldKnown)
	{
		g_confirmPending = false;
		g_explainUntilMs = 0;
	}
	else
	{
		watchForBlockedConfirm();
	}

	if (g_applied && t_worldKnown == g_open) return;
	g_applied = true;
	g_open = t_worldKnown;

	entries[NEW_GAME_ENTRY].m_nAction = t_worldKnown ? g_openAction[0] : MENUACTION_SKIP;
	entries[LOAD_GAME_ENTRY].m_nAction = t_worldKnown ? g_openAction[1] : MENUACTION_SKIP;

	if (t_worldKnown) refreshSaveSlotList();
}

void MenuGate::refreshSaveSlotList()
{
	reinterpret_cast<void(__cdecl*)()>(REFRESH_SAVE_SLOT_LIST)();
}

bool MenuGate::shouldExplainBlock()
{
	return g_explainUntilMs != 0 && GetTickCount() < g_explainUntilMs;
}
