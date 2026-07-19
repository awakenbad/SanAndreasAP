#include "TrapHandler.h"
#include "common.h"
#include "CStats.h"
#include "CClothes.h"
#include "CWanted.h"
#include "CAutomobile.h"
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

int TrapHandler::randomDurationSeconds() const
{
	return 30 + std::rand() % 91; // 30-120s
}

void TrapHandler::giveTrap(const std::string& t_trapType)
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

	if (now < m_tireTrapEnd && vehicle->m_nVehicleSubClass == VEHICLE_AUTOMOBILE)
	{
		CAutomobile* car = reinterpret_cast<CAutomobile*>(vehicle);
		for (int wheel = 0; wheel < 4; ++wheel)
		{
			// Status 1 = burst (eCarWheelStatus, not in plugin-sdk's headers - needs live
			// confirmation that tires visibly pop).
			car->m_damageManager.SetWheelStatus(wheel, 1);
		}
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
