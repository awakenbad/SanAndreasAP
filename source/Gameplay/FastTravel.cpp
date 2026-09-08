#include "FastTravel.h"

#include <C3dMarkers.h>
#include <CAudioEngine.h>
#include <CFont.h>
#include <CGame.h>
#include <CPad.h>
#include <CRGBA.h>
#include <CSprite2d.h>
#include <CTheScripts.h>
#include <CTimer.h>
#include <CVector.h>
#include <common.h>
#include <eScriptCommands.h>
#include <enums/eAudioEvents.h>
#include <extensions/Screen.h>
#include <extensions/ScriptCommands.h>
#include <vector>

#include "EdgeTriggeredKey.h"
#include "PlayerControl.h"
#include "Teleport.h"

namespace
{
	class FastTravelPoint
	{
	public:
		const char* name;
		float x, y, z;
		int areaCode;
		float arrivalX, arrivalY, arrivalZ;
		int arrivalArea;
		float arrivalHeading;
	};

	const FastTravelPoint TRAVEL_POINTS[] = {
		{ "Johnson's House",     2493.116f, -1710.396f, 1014.742f, 3,
		   2495.900f, -1707.450f, 1014.742f, 3,  0.000f },
		{ "Badlands Trailer",   -2035.709f, -2528.667f,   30.625f, 0,
		  -2039.780f, -2529.100f,   30.625f, 0,  0.371f },
		{ "Doherty Garage",     -2026.139f,   162.085f,   28.843f, 0,
		  -2021.036f,   156.067f,   28.652f, 0, -1.526f },
		{ "Four Dragons Casino", 2026.075f,  1019.243f,   10.820f, 0,
		   2027.043f,   996.058f,   10.820f, 0, -1.475f },
	};
	constexpr int POINT_COUNT = static_cast<int>(std::size(TRAVEL_POINTS));

	const CRGBA MARKER_COLOUR(175, 153, 239, 255);

	constexpr unsigned int MARKER_ID_BASE = 0xAF990000;
	constexpr float MARKER_SIZE = 1.4f;
	constexpr unsigned char MARKER_ALPHA = 120;
	constexpr unsigned short MARKER_PULSE_PERIOD = 2048;
	constexpr float MARKER_PULSE_FRACTION = 0.2f;
	constexpr short MARKER_ROTATE_RATE = 1;
	constexpr float GROUND_READ_TOLERANCE = 5.0f;
	constexpr short MAP_READ_NEVER = -32768;

	constexpr int FADE_MS = 1000;
	constexpr int HOLD_MS = 1000;
	constexpr int FADE_OUT = 0;
	constexpr int FADE_IN = 1;

	enum class TravelPhase
	{
		None,
		FadingOut,
		Holding,
		FadingIn,
	};

	TravelPhase g_phase = TravelPhase::None;
	unsigned int g_phaseStartedMs = 0;
	int g_pendingTarget = -1;

	constexpr float ACTIVATE_RADIUS_SQ = 1.0f;

	EdgeTriggeredKey g_upKey{ 'W' };
	EdgeTriggeredKey g_downKey{ 'S' };
	EdgeTriggeredKey g_selectKey{ VK_SPACE };
	EdgeTriggeredKey g_leaveKey{ VK_RETURN };
	EdgeTriggeredKey g_leaveKeyAlt{ 'F' };

	bool g_menuOpen = false;
	bool g_dismissed = false;
	int g_standingAt = -1;
	int g_selected = 0;

	const float MENU_WIDTH = 520.0f;
	const float MENU_ROW_HEIGHT = 40.0f;
	const float MENU_TITLE_HEIGHT = 46.0f;
	const float MENU_FOOTER_HEIGHT = 34.0f;
	const float MENU_PADDING = 14.0f;

	CVector positionOf(const FastTravelPoint& t_point)
	{
		return CVector(t_point.x, t_point.y, t_point.z);
	}

	CVector arrivalOf(const FastTravelPoint& t_point)
	{
		return CVector(t_point.arrivalX, t_point.arrivalY, t_point.arrivalZ);
	}

	void faceArrivalHeading(const FastTravelPoint& t_point)
	{
		CPlayerPed* player = FindPlayerPed();
		if (!player) return;

		player->SetHeading(t_point.arrivalHeading);
		player->m_fHeadingCurrent = t_point.arrivalHeading;
		player->m_fHeadingGoal = t_point.arrivalHeading;

		plugin::Command<eScriptCommands::COMMAND_SET_CAMERA_BEHIND_PLAYER>();
	}

	std::vector<int> destinations()
	{
		std::vector<int> list;
		for (int i = 0; i < POINT_COUNT; ++i)
		{
			if (i != g_standingAt) list.push_back(i);
		}
		return list;
	}

	int pointUnderPlayer()
	{
		CPlayerPed* player = FindPlayerPed();
		if (!player || player->bInVehicle) return -1;
		if (!PlayerControl::isInControl() || CTheScripts::IsPlayerOnAMission()) return -1;

		CVector playerPos = player->GetPosition();
		for (int i = 0; i < POINT_COUNT; ++i)
		{
			if (TRAVEL_POINTS[i].areaCode != CGame::currArea) continue;

			float dx = playerPos.x - TRAVEL_POINTS[i].x;
			float dy = playerPos.y - TRAVEL_POINTS[i].y;
			if (dx * dx + dy * dy < ACTIVATE_RADIUS_SQ) return i;
		}
		return -1;
	}

	void closeMenu()
	{
		g_menuOpen = false;
		CPad* pad = CPad::GetPad(0);
		if (pad) pad->DisablePlayerControls = false;
	}

	void handleMenuInput()
	{
		std::vector<int> list = destinations();
		if (list.empty())
		{
			closeMenu();
			return;
		}

		int count = static_cast<int>(list.size());
		if (g_selected >= count) g_selected = count - 1;

		if (g_upKey.justPressed())
		{
			g_selected = (g_selected + count - 1) % count;
			AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_HIGHLIGHT, 0.0f, 1.0f);
		}
		if (g_downKey.justPressed())
		{
			g_selected = (g_selected + 1) % count;
			AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_HIGHLIGHT, 0.0f, 1.0f);
		}

		if (g_leaveKey.justPressed() || g_leaveKeyAlt.justPressed())
		{
			AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_BACK, 0.0f, 1.0f);
			g_dismissed = true;
			closeMenu();
			return;
		}

		if (g_selectKey.justPressed())
		{
			AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_SELECT, 0.0f, 1.0f);
			g_pendingTarget = list[g_selected];
			g_dismissed = true;
			closeMenu();

			g_phase = TravelPhase::FadingOut;
			g_phaseStartedMs = CTimer::m_snTimeInMilliseconds;
			plugin::Command<eScriptCommands::COMMAND_DO_FADE>(FADE_MS, FADE_OUT);
		}
	}
}

void FastTravel::placeMarkers()
{
	if (g_phase != TravelPhase::None) return;

	for (int i = 0; i < POINT_COUNT; ++i)
	{
		if (TRAVEL_POINTS[i].areaCode != CGame::currArea) continue;
		if (g_menuOpen && i == g_standingAt) continue;

		CVector position = positionOf(TRAVEL_POINTS[i]);
		C3dMarker* marker = C3dMarkers::PlaceMarker(MARKER_ID_BASE + i,
			MARKER3D_CYLINDER, position, MARKER_SIZE,
			MARKER_COLOUR.r, MARKER_COLOUR.g, MARKER_COLOUR.b, MARKER_ALPHA,
			MARKER_PULSE_PERIOD, MARKER_PULSE_FRACTION, MARKER_ROTATE_RATE,
			0.0f, 0.0f, 1.0f, false);
		if (!marker) continue;

		if (marker->m_mat.GetPosition().z < position.z - GROUND_READ_TOLERANCE)
		{
			marker->m_nLastMapReadX = MAP_READ_NEVER;
			marker->m_nLastMapReadY = MAP_READ_NEVER;
		}
	}
}

namespace
{
	void advanceTravel()
	{
		CPad* pad = CPad::GetPad(0);
		if (pad) pad->DisablePlayerControls = true;

		unsigned int now = CTimer::m_snTimeInMilliseconds;
		unsigned int phaseLength = g_phase == TravelPhase::Holding ? HOLD_MS : FADE_MS;
		if (now - g_phaseStartedMs < phaseLength) return;

		if (g_phase == TravelPhase::FadingOut)
		{
			const FastTravelPoint& target = TRAVEL_POINTS[g_pendingTarget];
			Teleport::toExact(arrivalOf(target), false, target.arrivalArea);
			faceArrivalHeading(target);

			g_phase = TravelPhase::Holding;
			g_phaseStartedMs = now;
			return;
		}

		if (g_phase == TravelPhase::Holding)
		{
			g_phase = TravelPhase::FadingIn;
			g_phaseStartedMs = now;
			plugin::Command<eScriptCommands::COMMAND_DO_FADE>(FADE_MS, FADE_IN);
			return;
		}

		g_phase = TravelPhase::None;
		g_pendingTarget = -1;
		if (pad) pad->DisablePlayerControls = false;
	}
}

void FastTravel::update()
{
	if (g_phase != TravelPhase::None)
	{
		advanceTravel();
		return;
	}

	if (g_menuOpen)
	{
		if (!FindPlayerPed())
		{
			closeMenu();
			return;
		}

		CPad* pad = CPad::GetPad(0);
		if (pad) pad->DisablePlayerControls = true;
		handleMenuInput();
		return;
	}

	g_standingAt = pointUnderPlayer();

	if (g_standingAt < 0)
	{
		g_dismissed = false;
		return;
	}

	if (!g_dismissed)
	{
		AudioEngine.ReportFrontendAudioEvent(AE_FRONTEND_SELECT, 0.0f, 1.0f);
		g_menuOpen = true;
		g_selected = 0;

		g_upKey.prime();
		g_downKey.prime();
		g_selectKey.prime();
		g_leaveKey.prime();
		g_leaveKeyAlt.prime();
	}
}

void FastTravel::draw()
{
	if (!g_menuOpen) return;

	std::vector<int> list = destinations();
	if (list.empty()) return;

	CFont::SetFontStyle(FONT_SUBTITLES);
	CFont::SetProportional(true);
	CFont::SetOrientation(ALIGN_LEFT);
	CFont::SetDropShadowPosition(1);
	CFont::SetBackground(false, false);
	CFont::SetWrapx(static_cast<float>(RsGlobal.maximumWidth));

	float width = SCREEN_MULTIPLIER(MENU_WIDTH);
	float rowHeight = SCREEN_MULTIPLIER(MENU_ROW_HEIGHT);
	float padding = SCREEN_MULTIPLIER(MENU_PADDING);
	float titleHeight = SCREEN_MULTIPLIER(MENU_TITLE_HEIGHT);
	float footerHeight = SCREEN_MULTIPLIER(MENU_FOOTER_HEIGHT);
	float height = titleHeight + rowHeight * static_cast<float>(list.size()) + footerHeight
		+ padding * 2.0f;

	float left = (static_cast<float>(RsGlobal.maximumWidth) - width) * 0.5f;
	float top = (static_cast<float>(RsGlobal.maximumHeight) - height) * 0.5f;

	CSprite2d::DrawRect(CRect(left, top, left + width, top + height), CRGBA(0, 0, 0, 190));

	CFont::SetScale(SCREEN_MULTIPLIER(0.8f), SCREEN_MULTIPLIER(1.6f));
	CFont::SetColor(CRGBA(255, 255, 255, 255));
	CFont::PrintString(left + padding, top + padding, "FAST TRAVEL");

	CFont::SetScale(SCREEN_MULTIPLIER(0.65f), SCREEN_MULTIPLIER(1.3f));
	float rowY = top + padding + titleHeight;
	for (int i = 0; i < static_cast<int>(list.size()); ++i)
	{
		bool selected = i == g_selected;
		if (selected)
		{
			CSprite2d::DrawRect(CRect(left + padding * 0.5f, rowY,
				left + width - padding * 0.5f, rowY + rowHeight),
				CRGBA(MARKER_COLOUR.r, MARKER_COLOUR.g, MARKER_COLOUR.b, 90));
		}
		CFont::SetColor(selected ? CRGBA(255, 255, 255, 255) : CRGBA(190, 190, 190, 255));
		CFont::PrintString(left + padding, rowY + SCREEN_MULTIPLIER(4.0f), TRAVEL_POINTS[list[i]].name);
		rowY += rowHeight;
	}

	CFont::SetScale(SCREEN_MULTIPLIER(0.5f), SCREEN_MULTIPLIER(1.0f));
	CFont::SetColor(CRGBA(170, 170, 170, 255));
	CFont::PrintString(left + padding, rowY + SCREEN_MULTIPLIER(8.0f),
		"W/S select    SPACE travel    ENTER or F leave");
}
