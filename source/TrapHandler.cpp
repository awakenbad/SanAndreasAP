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
#include "ScreenScale.h"
#include <CFont.h>
#include <CRGBA.h>
#include <CSprite2d.h>
#include <cstdlib>
#include <vector>

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
}

int TrapHandler::randomDurationSeconds() const
{
	return 30 + std::rand() % 91; // 30-120s
}

void TrapHandler::save(SaveDataManager& t_saveData)
{
	// Seconds left rather than an end time: steady_clock's epoch is arbitrary per process, so an
	// absolute time point means nothing once the game restarts.
	int remainingSeconds = 0;
	if (m_fatTrapActive)
	{
		Clock::duration remaining = m_fatTrapEnd - Clock::now();
		if (remaining > Clock::duration::zero())
		{
			remainingSeconds = static_cast<int>(std::chrono::duration_cast<std::chrono::seconds>(remaining).count());
		}
	}

	t_saveData.setValue(FAT_ACTIVE_KEY, m_fatTrapActive ? "1" : "0");
	t_saveData.setValue(FAT_SAVED_FAT_KEY, std::to_string(m_savedFat));
	t_saveData.setValue(FAT_SAVED_MUSCLE_KEY, std::to_string(m_savedMuscle));
	t_saveData.setValue(FAT_REMAINING_KEY, std::to_string(remainingSeconds));
}

void TrapHandler::load(const SaveDataManager& t_saveData)
{
	m_fatTrapActive = t_saveData.getValue(FAT_ACTIVE_KEY, "0") == "1";

	if (!m_fatTrapActive)
	{
		// Any trap still running belonged to whichever save we came from. Abandon it WITHOUT
		// restoring: this save's stats are its own, and writing another save's pre-trap values
		// over them would quietly rewrite CJ's build.
		m_fatTrapEnd = Clock::time_point{};
		return;
	}

	m_savedFat = parseFloatOr(t_saveData.getValue(FAT_SAVED_FAT_KEY, "0"), 0.0f);
	m_savedMuscle = parseFloatOr(t_saveData.getValue(FAT_SAVED_MUSCLE_KEY, "0"), 0.0f);

	// Resume with what was left, so quitting and reloading isn't a way to shrug the trap off.
	// update() restores the stats the moment this expires, which is also what makes "fat forever"
	// impossible now - even a zero or malformed value just restores on the next tick.
	int remainingSeconds = parseIntOr(t_saveData.getValue(FAT_REMAINING_KEY, "0"), 0);
	m_fatTrapEnd = Clock::now() + std::chrono::seconds(remainingSeconds);
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
		m_tireTrapEnd = Clock::now() + std::chrono::seconds(randomDurationSeconds());
	}
	else if (t_trapType == "fat")
	{
		if (!m_fatTrapActive)
		{
			m_savedFat = CStats::GetStatValue(STAT_FAT);
			m_savedMuscle = CStats::GetStatValue(STAT_MUSCLE);
			m_fatTrapActive = true;
		}
		m_fatTrapEnd = Clock::now() + std::chrono::seconds(randomDurationSeconds());
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
			unsigned int newLevel = wanted->m_nWantedLevel + 1;
			if (newLevel > 6) newLevel = 6;
			wanted->SetWantedLevelNoDrop(static_cast<int>(newLevel));
		}
	}
	else if (t_trapType == "carfire")
	{
		m_carFirePending = true;
	}
}

void TrapHandler::update()
{
	// Release anything that arrived mid-cutscene, in the order it arrived. Their timers start now
	// rather than when they were received, which is what the player will perceive as fair.
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

	if (m_fatTrapActive && now >= m_fatTrapEnd)
	{
		m_fatTrapActive = false;
		CStats::SetStatValue(STAT_FAT, m_savedFat);
		CStats::SetStatValue(STAT_MUSCLE, m_savedMuscle);
		if (player)
		{
			CClothes::RebuildPlayer(player, false);
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
		// Engine health below 250 ignites the engine fire without an instant explosion.
		if (vehicle->m_fHealth > 240.0f)
		{
			vehicle->m_fHealth = 240.0f;
		}
	}
}

// Where a vehicle keeps its wheel status depends on which C++ class it actually is, so this
// branches on m_nVehicleClass - the base type - rather than the subtype. Getting that wrong is not
// a missed effect but memory corruption: a CBike has no CDamageManager, so reaching for one
// through a CAutomobile pointer writes into whatever happens to sit at that offset.
void TrapHandler::burstTires(CVehicle* t_vehicle)
{
	// Status 1 = burst (eCarWheelStatus, absent from plugin-sdk's headers). Confirmed live on
	// cars; bikes use the same values in their own array.
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
		// Bikes carry wheel status on the vehicle itself and have no damage manager at all, which
		// is why the old CAutomobile-only check could never have reached them.
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
	if (lines.empty()) return;

	float scale = ScreenScale::factor();
	float x = static_cast<float>(RsGlobal.maximumWidth) - ScreenScale::of(30.0f);
	float y = ScreenScale::of(250.0f); // below the clock/money/health HUD block and wanted stars

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
