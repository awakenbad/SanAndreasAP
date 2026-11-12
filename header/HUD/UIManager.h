#pragma once
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"
#include <d3d9.h>
#include <RenderWare.h>

#include "EdgeTriggeredKey.h"
#include "SettingsMenu.h"
#include "ItemHistory.h"

class UIManager
{
public:
	UIManager() = default;
	void init();
	void update(const ItemHistory& t_itemHistory);
	void toggleWindow();
	void cleanUp();

private:
	SettingsMenu m_settingsMenu;
	EdgeTriggeredKey m_toggleVisible{ VK_F9 };
	bool test = false;
};

