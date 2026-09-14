#include "PlayerControl.h"
#include "common.h"
#include "RunningScripts.h"
#include "ScriptGlobals.h"
#include <CPad.h>
#include <CCutsceneMgr.h>
#include <cmath>

namespace
{
	constexpr char HIGH_STAKES_SCRIPT[] = "CESAR1";
	constexpr int CESAR_MARKER_X = 480;
	constexpr int CESAR_MARKER_Y = 481;
	constexpr int CESAR_MARKER_Z = 482;
	constexpr float MARKER_HORIZONTAL_RADIUS = 14.0f;
	constexpr float MARKER_VERTICAL_RADIUS = 6.0f;
}

bool PlayerControl::isInControl()
{
	if (!FindPlayerPed()) return false;
	if (CCutsceneMgr::ms_running) return false;
	CPad* pad = CPad::GetPad(0);
	if (pad && pad->DisablePlayerControls != 0) return false;

	return true;
}

bool PlayerControl::isInHighStakesDriveway()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return false;
	if (!RunningScripts::isActive(HIGH_STAKES_SCRIPT)) return false;

	CVector position = player->bInVehicle && player->m_pVehicle
		? player->m_pVehicle->GetPosition()
		: player->GetPosition();

	return std::fabs(position.x - ScriptGlobals::readFloat(CESAR_MARKER_X)) <= MARKER_HORIZONTAL_RADIUS
		&& std::fabs(position.y - ScriptGlobals::readFloat(CESAR_MARKER_Y)) <= MARKER_HORIZONTAL_RADIUS
		&& std::fabs(position.z - ScriptGlobals::readFloat(CESAR_MARKER_Z)) <= MARKER_VERTICAL_RADIUS;
}
