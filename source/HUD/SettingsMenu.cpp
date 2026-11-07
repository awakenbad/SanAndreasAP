#include "SettingsMenu.h"
#include "ModSettings.h"
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
		if (ImGui::ColorEdit3(label, rgb))
		{
			colour = CRGBA(static_cast<unsigned char>(rgb[0] * 255.0f + 0.5f),
				static_cast<unsigned char>(rgb[1] * 255.0f + 0.5f),
				static_cast<unsigned char>(rgb[2] * 255.0f + 0.5f), colour.a);
		}
	}
}

void SettingsMenu::draw()
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
