#pragma once

class ItemHistory;

class SettingsMenu
{
public:
	void draw(const ItemHistory& t_itemHistory);

private:
	void drawSettingsTab();
	void drawItemLogTab(const ItemHistory& t_itemHistory);

	bool m_unsaved = false;
};
