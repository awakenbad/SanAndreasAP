#include "TrapHandler.h"
#include "PlayerControl.h"
#include "SaveDataManager.h"
#include "ParseUtils.h"
#include "common.h"
#include "CStats.h"
#include "CClothes.h"
#include "CWanted.h"
#include "CAutomobile.h"
#include "CBike.h"
#include "CWeather.h"
#include <CFont.h>
#include <CRGBA.h>
#include <CSprite2d.h>
#include <random>
#include <vector>
#include <plugin.h>

static std::string formatRemaining(std::chrono::steady_clock::duration t_remaining)
{
	int totalSeconds = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(t_remaining).count()) + 1;
	int minutes = totalSeconds / 60;
	int seconds = totalSeconds % 60;
	return std::to_string(minutes) + ":" + (seconds < 10 ? "0" : "") + std::to_string(seconds);
}

namespace
{
	constexpr char FAT_ACTIVE_KEY[] = "trap_fat_active";
	constexpr char FAT_SAVED_FAT_KEY[] = "trap_fat_saved_fat";
	constexpr char FAT_SAVED_MUSCLE_KEY[] = "trap_fat_saved_muscle";
	constexpr char FAT_REMAINING_KEY[] = "trap_fat_remaining_seconds";

	constexpr char WEATHER_ACTIVE_KEY[] = "trap_weather_active";
	constexpr char WEATHER_FORCED_KEY[] = "trap_weather_forced_type";
	constexpr char WEATHER_REMAINING_KEY[] = "trap_weather_remaining_seconds";

	constexpr short BAD_WEATHER_TYPES[] = { WEATHER_RAINY_SF, WEATHER_FOGGY_SF, WEATHER_SANDSTORM_DESERT };

	constexpr short WEATHER_NOT_FORCED = -1;

	int randomBetween(int t_low, int t_high)
	{
		static std::mt19937 generator{ std::random_device{}() };
		return std::uniform_int_distribution<int>(t_low, t_high)(generator);
	}

	int remainingSeconds(bool t_active, std::chrono::steady_clock::time_point t_end)
	{
		if (!t_active) return 0;
		auto remaining = t_end - std::chrono::steady_clock::now();
		if (remaining <= std::chrono::steady_clock::duration::zero()) return 0;
		return static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(remaining).count());
	}
}

int TrapHandler::randomSeconds(int t_low, int t_high) const
{
	return randomBetween(t_low, t_high);
}

int TrapHandler::randomWantedStars() const
{
	int maxStars = static_cast<int>(CWanted::MaximumWantedLevel);
	if (maxStars <= WANTED_TRAP_MIN_STARS) return maxStars;

	return randomBetween(WANTED_TRAP_MIN_STARS, maxStars);
}

void TrapHandler::save(SaveDataManager& t_saveData)
{
	t_saveData.setValue(FAT_ACTIVE_KEY, m_fatTrapActive ? "1" : "0");
	t_saveData.setValue(FAT_SAVED_FAT_KEY, std::to_string(m_savedFat));
	t_saveData.setValue(FAT_SAVED_MUSCLE_KEY, std::to_string(m_savedMuscle));
	t_saveData.setValue(FAT_REMAINING_KEY, std::to_string(remainingSeconds(m_fatTrapActive, m_fatTrapEnd)));

	t_saveData.setValue(WEATHER_ACTIVE_KEY, m_weatherTrapActive ? "1" : "0");
	t_saveData.setValue(WEATHER_FORCED_KEY, std::to_string(m_forcedWeather));
	t_saveData.setValue(WEATHER_REMAINING_KEY, std::to_string(remainingSeconds(m_weatherTrapActive, m_weatherTrapEnd)));
}

void TrapHandler::load(const SaveDataManager& t_saveData)
{
	m_fatTrapActive = t_saveData.getValue(FAT_ACTIVE_KEY, "0") == "1";
	m_fatTrapEnd = Clock::time_point{};

	if (m_fatTrapActive)
	{
		m_savedFat = parseFloatOr(t_saveData.getValue(FAT_SAVED_FAT_KEY, "0"), 0.0f);
		m_savedMuscle = parseFloatOr(t_saveData.getValue(FAT_SAVED_MUSCLE_KEY, "0"), 0.0f);
		m_fatTrapEnd = Clock::now()
			+ std::chrono::seconds(parseIntOr(t_saveData.getValue(FAT_REMAINING_KEY, "0"), 0));
	}

	m_weatherTrapActive = t_saveData.getValue(WEATHER_ACTIVE_KEY, "0") == "1";
	m_weatherTrapEnd = Clock::time_point{};

	if (m_weatherTrapActive)
	{
		m_forcedWeather = static_cast<short>(parseIntOr(t_saveData.getValue(WEATHER_FORCED_KEY, "0"), 0));
		m_weatherTrapEnd = Clock::now()
			+ std::chrono::seconds(parseIntOr(t_saveData.getValue(WEATHER_REMAINING_KEY, "0"), 0));
	}
}

void TrapHandler::giveTrap(const std::string& t_trapType)
{
	if (!PlayerControl::isInControl())
	{
		m_deferredTraps.push_back(t_trapType);
		return;
	}

	applyTrap(t_trapType);
}

void TrapHandler::applyTrap(const std::string& t_trapType)
{
	if (t_trapType == "tires")
	{
		m_tireTrapEnd = Clock::now() + std::chrono::seconds(randomSeconds(TRAP_MIN_SECONDS, TRAP_MAX_SECONDS));
	}
	else if (t_trapType == "fat")
	{
		if (!m_fatTrapActive)
		{
			m_savedFat = CStats::GetStatValue(STAT_FAT);
			m_savedMuscle = CStats::GetStatValue(STAT_MUSCLE);
			m_fatTrapActive = true;
		}
		m_fatTrapEnd = Clock::now() + std::chrono::seconds(randomSeconds(TRAP_MIN_SECONDS, TRAP_MAX_SECONDS));
		CStats::SetStatValue(STAT_FAT, 1000.0f);
		CStats::SetStatValue(STAT_MUSCLE, 0.0f);
		if (CPlayerPed* player = FindPlayerPed())
		{
			CClothes::RebuildPlayer(player, false);
		}
	}
	else if (t_trapType == "wanted")
	{
		CPlayerPed* player = FindPlayerPed();
		CWanted* wanted = player ? player->GetWanted() : nullptr;
		if (wanted)
		{
			unsigned int maxLevel = CWanted::MaximumWantedLevel;
			unsigned int newLevel = wanted->m_nWantedLevel + randomWantedStars();
			if (newLevel > maxLevel) newLevel = maxLevel;
			wanted->SetWantedLevelNoDrop(static_cast<int>(newLevel));
		}
	}
	else if (t_trapType == "carfire")
	{
		m_carFirePending = true;
	}
	else if (t_trapType == "weather")
	{
		startWeatherTrap(BAD_WEATHER_TYPES[randomBetween(0, static_cast<int>(std::size(BAD_WEATHER_TYPES)) - 1)]);
	}
}

void TrapHandler::startWeatherTrap(short t_weather)
{
	m_forcedWeather = t_weather;
	CWeather::ForceWeatherNow(m_forcedWeather);
	m_weatherTrapActive = true;
	m_weatherTrapEnd = Clock::now()
		+ std::chrono::seconds(randomSeconds(WEATHER_MIN_SECONDS, WEATHER_MAX_SECONDS));
}

void TrapHandler::update()
{
	if (!m_deferredTraps.empty() && PlayerControl::isInControl())
	{
		std::vector<std::string> toApply;
		toApply.swap(m_deferredTraps);
		for (const std::string& trapType : toApply)
		{
			applyTrap(trapType);
		}
	}

	CPlayerPed* player = FindPlayerPed();
	Clock::time_point now = Clock::now();

	if (m_fatTrapActive)
	{
		if (now >= m_fatTrapEnd)
		{
			m_fatTrapActive = false;
			CStats::SetStatValue(STAT_FAT, m_savedFat);
			CStats::SetStatValue(STAT_MUSCLE, m_savedMuscle);
			if (player)
			{
				CClothes::RebuildPlayer(player, false);
			}
		}
		else if (CStats::GetStatValue(STAT_MUSCLE) > m_savedMuscle)
		{
			m_savedMuscle = CStats::GetStatValue(STAT_MUSCLE);
			CStats::SetStatValue(STAT_MUSCLE, 0.0f);
			if (player)
			{
				CClothes::RebuildPlayer(player, false);
			}
		}
	}

	if (m_weatherTrapActive)
	{
		if (now >= m_weatherTrapEnd)
		{
			m_weatherTrapActive = false;
			if (CWeather::ForcedWeatherType == m_forcedWeather)
			{
				CWeather::ReleaseWeather();
				CWeather::SetWeatherToAppropriateTypeNow();
			}
		}
		else if (CWeather::ForcedWeatherType == WEATHER_NOT_FORCED)
		{
			CWeather::ForceWeatherNow(m_forcedWeather);
		}
	}

	if (!player) return;
	CVehicle* vehicle = player->bInVehicle ? player->m_pVehicle : nullptr;
	if (!vehicle) return;

	if (now < m_tireTrapEnd)
	{
		burstTires(vehicle);
	}

	if (m_carFirePending)
	{
		m_carFirePending = false;
		if (vehicle->m_fHealth > 240.0f)
		{
			vehicle->m_fHealth = 240.0f;
		}
	}
}

void TrapHandler::burstTires(CVehicle* t_vehicle)
{
	const int WHEEL_STATUS_BURST = 1;

	if (t_vehicle->m_nVehicleClass == VEHICLE_AUTOMOBILE)
	{
		CAutomobile* car = reinterpret_cast<CAutomobile*>(t_vehicle);
		for (int wheel = 0; wheel < 4; ++wheel)
		{
			car->m_damageManager.SetWheelStatus(wheel, WHEEL_STATUS_BURST);
		}
	}
	else if (t_vehicle->m_nVehicleClass == VEHICLE_BIKE)
	{
		CBike* bike = reinterpret_cast<CBike*>(t_vehicle);
		for (int wheel = 0; wheel < CBike::NUM_WHEELS; ++wheel)
		{
			bike->m_nWheelStatus[wheel] = static_cast<uint8_t>(WHEEL_STATUS_BURST);
		}
	}
}

void TrapHandler::drawTimers() const
{
	Clock::time_point now = Clock::now();

	std::vector<std::string> lines;
	if (now < m_tireTrapEnd)
	{
		lines.push_back("Flat Tires Trap: " + formatRemaining(m_tireTrapEnd - now));
	}
	if (m_fatTrapActive && now < m_fatTrapEnd)
	{
		lines.push_back("Fat CJ Trap: " + formatRemaining(m_fatTrapEnd - now));
	}
	if (m_weatherTrapActive && now < m_weatherTrapEnd)
	{
		lines.push_back("Bad Weather Trap: " + formatRemaining(m_weatherTrapEnd - now));
	}
	if (lines.empty()) return;

	float scale = SCREEN_MULTIPLIER(1.0f);
	float x = static_cast<float>(RsGlobal.maximumWidth) - SCREEN_MULTIPLIER(30.0f);
	float y = SCREEN_MULTIPLIER(250.0f);

	for (const std::string& line : lines)
	{
		CFont::SetFontStyle(FONT_SUBTITLES);
		CFont::SetScale(0.7f * scale, 1.4f * scale);
		CFont::SetColor(CRGBA(255, 255, 255, 255));
		CFont::SetProportional(true);
		CFont::SetOrientation(ALIGN_RIGHT);
		CFont::SetDropShadowPosition(1);
		CFont::SetBackground(false, false);
		CFont::SetRightJustifyWrap(0.0f);

		float textWidth = CFont::GetStringWidth(const_cast<char*>(line.c_str()), true);
		CRect box(x - textWidth - 5.0f * scale, y, x + 5.0f * scale, y + 30.0f * scale);
		CSprite2d::DrawRect(box, CRGBA(0, 0, 0, 150));

		CFont::PrintString(x, y, line.c_str());
		y += 40.0f * scale;
	}
}
