#include "ModSettings.h"
#include <windows.h>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <string>
#include <plugin.h>

namespace
{
	constexpr char DISPLAY_SECTION[] = "Display";
	constexpr char COLOURS_SECTION[] = "Colors";
	constexpr char GAMEPLAY_SECTION[] = "Gameplay";

	constexpr char NOTIFICATION_KEY[] = "NotificationSeconds";
	constexpr char NUMBER_SCALE_KEY[] = "CollectibleNumberScale";
	constexpr char COUNTER_SCALE_KEY[] = "MissionCounterScale";
	constexpr char DIM_KEY[] = "UnselectedDim";
	constexpr char FAST_TRAVEL_KEY[] = "FastTravel";
	constexpr char COLLECTIBLE_HEIGHT_INDICATOR_KEY[] = "CollectibleHeightIndicator";
	constexpr char DISABLE_HORSE_BETTING_KEY[] = "DisableHorseBetting";

	constexpr float NOTIFICATION_DEFAULT = 8.0f;
	constexpr float NOTIFICATION_MIN = 1.0f;
	constexpr float NOTIFICATION_MAX = 60.0f;

	constexpr float SCALE_DEFAULT = 0.5f;
	constexpr float SCALE_MIN = 0.1f;
	constexpr float SCALE_MAX = 2.0f;

	constexpr int FAST_TRAVEL_DEFAULT = 1;
	constexpr int COLLECTIBLE_HEIGHT_INDICATOR_DEFAULT = (int) ModSettings::CollectibleHeightIndicator::OnBlip;
	constexpr int DISABLE_HORSE_BETTING_DEFAULT = 0;

	constexpr int DIM_DEFAULT = 60;
	constexpr int DIM_MIN = 10;
	constexpr int DIM_MAX = 100;

	constexpr int COLOUR_COUNT = static_cast<int>(ModSettings::ItemColour::Count);

	const char* const COLOUR_KEYS[COLOUR_COUNT] = { "Progression", "Useful", "Trap", "Filler" };
	constexpr unsigned long COLOUR_DEFAULTS[COLOUR_COUNT] = { 0xAF99EF, 0x6D8BE8, 0xFA8072, 0x00EEEE };

	constexpr char TEMPLATE_TEXT[] =
		"; GTA: San Andreas Archipelago - mod settings.\r\n"
		"; Delete the file to regenerate it.\r\n"
		"\r\n"
		"[Display]\r\n"
		"; How long a notification stays on screen before fading, in seconds (range = 1-60, default = 8).\r\n"
		"; Note: if over 5 items are received at the same time, this setting is ignored and only applied to the last 5 notifications.\r\n"
		"NotificationSeconds=8\r\n"
		"\r\n"
		"; Size of the numbers beside collectible blips on the radar (0.1 - 2, default 0.5).\r\n"
		"CollectibleNumberScale=0.5\r\n"
		"\r\n"
		"; Size of the Progressive Mission counter drawn on radar mission blips (0.1 - 2, default 0.5).\r\n"
		"MissionCounterScale=0.5\r\n"
		"\r\n"
		"[Colors]\r\n"
		"; Ammu-Nation row colours by item importance, as RRGGBB hex.\r\n"
		"; The defaults match the Archipelago text client's palette.\r\n"
		"Progression=AF99EF\r\n"
		"Useful=6D8BE8\r\n"
		"Trap=FA8072\r\n"
		"Filler=00EEEE\r\n"
		"\r\n"
		"; How dim an unselected row is, as a percentage of its colour (10 - 100). The selected row\r\n"
		"; always draws at full strength, so a lower number makes the selection stand out more.\r\n"
		"UnselectedDim=60\r\n"
		"\r\n"
		"[Gameplay]\r\n"
		"; Fast travel markers at Johnson's House, Badlands trailer, Doherty\r\n"
		"; garage and Four Dragons casino (1 = on, 0 = off, default = 1).\r\n"
		"FastTravel=1\r\n"
		"\r\n"
		"; Shows a height indicator (Above △, Below ▽, Same Height □) for collectibles when you're\r\n"
		"; close enough to them (0 = off, 1 = on top of blip, 2 = alternating with number, default = 1).\r\n"
		"CollectibleHeightIndicator=1\r\n"
		"\r\n"
		"; Disables the horse betting machines"
		"; (0 = machines are active, 1 = machines are inactive, default = 0)\r\n"
		"DisableHorseBetting=0\r\n";

	float g_notificationSeconds = NOTIFICATION_DEFAULT;
	float g_collectibleNumberScale = SCALE_DEFAULT;
	float g_missionCounterScale = SCALE_DEFAULT;
	int g_unselectedDimPercent = DIM_DEFAULT;
	bool g_fastTravelEnabled = FAST_TRAVEL_DEFAULT != 0;
	int g_collectibleHeightIndicatorMode = COLLECTIBLE_HEIGHT_INDICATOR_DEFAULT;
	bool g_disableHorseBetting = DISABLE_HORSE_BETTING_DEFAULT != 0;
	CRGBA g_itemColours[COLOUR_COUNT];

	std::string settingsPath()
	{
		HMODULE module = nullptr;
		if (!GetModuleHandleExA(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			reinterpret_cast<LPCSTR>(&settingsPath), &module))
		{
			return {};
		}

		char path[MAX_PATH]{};
		if (GetModuleFileNameA(module, path, MAX_PATH) == 0) return {};

		return std::filesystem::path(path).replace_extension(".ini").string();
	}

	bool readRaw(const std::string& t_path, const char* t_section, const char* t_key, char* t_buffer, size_t t_size)
	{
		return GetPrivateProfileStringA(t_section, t_key, "", t_buffer, static_cast<DWORD>(t_size), t_path.c_str()) != 0;
	}

	float readFloat(const std::string& t_path, const char* t_key, float t_default, float t_low, float t_high)
	{
		char buffer[64]{};
		if (!readRaw(t_path, DISPLAY_SECTION, t_key, buffer, sizeof(buffer))) return t_default;

		char* end = nullptr;
		float value = std::strtof(buffer, &end);
		if (end == buffer || *end != '\0') return t_default;

		if (value < t_low) return t_low;
		if (value > t_high) return t_high;
		return value;
	}

	int readInt(const std::string& t_path, const char* t_section, const char* t_key, int t_default, int t_low, int t_high)
	{
		char buffer[64]{};
		if (!readRaw(t_path, t_section, t_key, buffer, sizeof(buffer))) return t_default;

		char* end = nullptr;
		long value = std::strtol(buffer, &end, 10);
		if (end == buffer || *end != '\0') return t_default;

		if (value < t_low) return t_low;
		if (value > t_high) return t_high;
		return static_cast<int>(value);
	}

	CRGBA unpack(unsigned long t_packed)
	{
		return CRGBA(static_cast<unsigned char>((t_packed >> 16) & 0xFF),
			static_cast<unsigned char>((t_packed >> 8) & 0xFF),
			static_cast<unsigned char>(t_packed & 0xFF), 255);
	}

	CRGBA readColour(const std::string& t_path, const char* t_key, unsigned long t_default)
	{
		char buffer[64]{};
		if (!readRaw(t_path, COLOURS_SECTION, t_key, buffer, sizeof(buffer))) return unpack(t_default);

		const char* text = buffer[0] == '#' ? buffer + 1 : buffer;
		char* end = nullptr;
		unsigned long value = std::strtoul(text, &end, 16);
		if (end != text + 6 || *end != '\0') return unpack(t_default);

		return unpack(value);
	}
}

void ModSettings::load()
{
	for (int i = 0; i < COLOUR_COUNT; ++i)
	{
		g_itemColours[i] = unpack(COLOUR_DEFAULTS[i]);
	}

	std::string path = settingsPath();
	if (path.empty()) return;

	if (GetFileAttributesA(path.c_str()) == INVALID_FILE_ATTRIBUTES)
	{
		FILE* file = nullptr;
		if (fopen_s(&file, path.c_str(), "wb") == 0 && file)
		{
			fwrite(TEMPLATE_TEXT, 1, sizeof(TEMPLATE_TEXT) - 1, file);
			fclose(file);
		}
	}

	g_notificationSeconds = readFloat(path, NOTIFICATION_KEY, NOTIFICATION_DEFAULT,
		NOTIFICATION_MIN, NOTIFICATION_MAX);
	g_collectibleNumberScale = readFloat(path, NUMBER_SCALE_KEY, SCALE_DEFAULT, SCALE_MIN, SCALE_MAX);
	g_missionCounterScale = readFloat(path, COUNTER_SCALE_KEY, SCALE_DEFAULT, SCALE_MIN, SCALE_MAX);

	for (int i = 0; i < COLOUR_COUNT; ++i)
	{
		g_itemColours[i] = readColour(path, COLOUR_KEYS[i], COLOUR_DEFAULTS[i]);
	}
	g_unselectedDimPercent = readInt(path, COLOURS_SECTION, DIM_KEY, DIM_DEFAULT, DIM_MIN, DIM_MAX);

	g_fastTravelEnabled = readInt(path, GAMEPLAY_SECTION, FAST_TRAVEL_KEY, FAST_TRAVEL_DEFAULT, 0, 1) != 0;
	g_collectibleHeightIndicatorMode = readInt(path, GAMEPLAY_SECTION, COLLECTIBLE_HEIGHT_INDICATOR_KEY, COLLECTIBLE_HEIGHT_INDICATOR_DEFAULT, 0, 2);
	g_disableHorseBetting = readInt(path, GAMEPLAY_SECTION, DISABLE_HORSE_BETTING_KEY, DISABLE_HORSE_BETTING_DEFAULT, 0, 1) != 0;
}

float ModSettings::notificationSeconds()
{
	return g_notificationSeconds;
}

float ModSettings::collectibleNumberScale()
{
	return SCREEN_MULTIPLIER(g_collectibleNumberScale);
}

float ModSettings::missionCounterScale()
{
	return SCREEN_MULTIPLIER(g_missionCounterScale);
}

const CRGBA& ModSettings::itemColour(ItemColour t_which)
{
	return g_itemColours[static_cast<int>(t_which)];
}

int ModSettings::unselectedDimPercent()
{
	return g_unselectedDimPercent;
}

bool ModSettings::fastTravelEnabled()
{
	return g_fastTravelEnabled;
}

ModSettings::CollectibleHeightIndicator ModSettings::collectibleHeightIndicatorMode()
{
	return (ModSettings::CollectibleHeightIndicator) g_collectibleHeightIndicatorMode;
}

bool ModSettings::horseBettingDisabled()
{
	return g_disableHorseBetting;
}