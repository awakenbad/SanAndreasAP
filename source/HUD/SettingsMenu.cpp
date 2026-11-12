#include "SettingsMenu.h"
#include "ModSettings.h"
#include "ItemHistory.h"
#include "imgui.h"
#include <plugin.h>

namespace
{
	const float FONT_SIZE = 16.0f;
	const float ITEM_WIDTH = 300.0f;
	const float BACKGROUND_ALPHA = 0.85f;

	const char* const HEIGHT_INDICATOR_NAMES[] = { "Off", "On top of blip", "Alternating with number" };

	void colourEdit(const char* label, CRGBA& colour)
	{
		float rgb[3] = { colour.r / 255.0f, colour.g / 255.0f, colour.b / 255.0f };
		if (ImGui::ColorEdit3(label, rgb, ImGuiColorEditFlags_NoInputs))
		{
			colour = CRGBA(static_cast<unsigned char>(rgb[0] * 255.0f + 0.5f),
				static_cast<unsigned char>(rgb[1] * 255.0f + 0.5f),
				static_cast<unsigned char>(rgb[2] * 255.0f + 0.5f), colour.a);
		}
	}

	ImVec4 toImColour(const CRGBA& t_colour)
	{
		return ImVec4(t_colour.r / 255.0f, t_colour.g / 255.0f, t_colour.b / 255.0f, 1.0f);
	}
}

void SettingsMenu::draw(const ItemHistory& t_itemHistory)
{
	ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
	ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
	ImGui::SetNextWindowBgAlpha(BACKGROUND_ALPHA);
	ImGui::PushFont(nullptr, SCREEN_MULTIPLIER(FONT_SIZE));
	ImGui::Begin("Archipelago", nullptr,
		ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);
	ImGui::PushItemWidth(SCREEN_MULTIPLIER(ITEM_WIDTH));

	if (ImGui::BeginTabBar("Tabs"))
	{
		if (ImGui::BeginTabItem("Settings"))
		{
			drawSettingsTab();
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Item log"))
		{
			drawItemLogTab(t_itemHistory);
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}

	ImGui::PopItemWidth();
	ImGui::End();
	ImGui::PopFont();
}

void SettingsMenu::drawSettingsTab()
{
	ModSettings::Values& values = ModSettings::values();

	ImGui::SeparatorText("Display");
	ImGui::SliderFloat("Notification duration (seconds)", &values.notificationSeconds,
		ModSettings::NOTIFICATION_MIN, ModSettings::NOTIFICATION_MAX, "%.0f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Collectible number size", &values.collectibleNumberScale,
		ModSettings::SCALE_MIN, ModSettings::SCALE_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp);
	ImGui::SliderFloat("Mission counter size", &values.missionCounterScale,
		ModSettings::SCALE_MIN, ModSettings::SCALE_MAX, "%.2f", ImGuiSliderFlags_AlwaysClamp);

	ImGui::SeparatorText("Ammu-Nation colours");
	colourEdit("Progression", values.itemColours[static_cast<int>(ModSettings::ItemColour::Progression)]);
	colourEdit("Useful", values.itemColours[static_cast<int>(ModSettings::ItemColour::Useful)]);
	colourEdit("Trap", values.itemColours[static_cast<int>(ModSettings::ItemColour::Trap)]);
	colourEdit("Filler", values.itemColours[static_cast<int>(ModSettings::ItemColour::Filler)]);
	ImGui::SliderInt("Unselected row brightness (%)", &values.unselectedDimPercent,
		ModSettings::DIM_MIN, ModSettings::DIM_MAX, "%d", ImGuiSliderFlags_AlwaysClamp);

	ImGui::SeparatorText("Gameplay");
	ImGui::Checkbox("Fast travel", &values.fastTravelEnabled);
	ImGui::Combo("Collectible height indicator", &values.collectibleHeightIndicator,
		HEIGHT_INDICATOR_NAMES, IM_ARRAYSIZE(HEIGHT_INDICATOR_NAMES));
}

void SettingsMenu::drawItemLogTab(const ItemHistory& t_itemHistory)
{
	const std::map<int, ItemHistoryEntry>& entries = t_itemHistory.entries();
	if (entries.empty())
	{
		ImGui::TextDisabled("No items received yet.");
		return;
	}

	ImGui::Text("%d items received", static_cast<int>(entries.size()));

	const ImGuiTableFlags flags = ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersInnerV
		| ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingStretchProp;
	if (!ImGui::BeginTable("ItemLog", 5, flags, ImVec2(0.0f, ImGui::GetContentRegionAvail().y))) return;

	ImGui::TableSetupScrollFreeze(0, 1);
	ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
	ImGui::TableSetupColumn("Item", ImGuiTableColumnFlags_WidthStretch, 3.0f);
	ImGui::TableSetupColumn("From", ImGuiTableColumnFlags_WidthStretch, 2.0f);
	ImGui::TableSetupColumn("Game", ImGuiTableColumnFlags_WidthStretch, 2.0f);
	ImGui::TableSetupColumn("Location", ImGuiTableColumnFlags_WidthStretch, 3.0f);
	ImGui::TableHeadersRow();

	for (auto it = entries.rbegin(); it != entries.rend(); ++it)
	{
		const ItemHistoryEntry& entry = it->second;
		ImGui::TableNextRow();
		ImGui::TableNextColumn();
		ImGui::TextDisabled("%d", entry.index + 1);
		ImGui::TableNextColumn();
		ImGui::TextColored(toImColour(ModSettings::itemColourForFlags(entry.flags)), "%s", entry.item.c_str());
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(entry.sender.c_str());
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(entry.game.c_str());
		ImGui::TableNextColumn();
		ImGui::TextUnformatted(entry.location.c_str());
	}

	ImGui::EndTable();
}
