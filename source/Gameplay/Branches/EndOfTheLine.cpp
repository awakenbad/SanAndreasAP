#include "EndOfTheLine.h"
#include "Marker.h"
#include "BranchProgress.h"
#include "ScriptGlobals.h"
#include "ScriptCommandHook.h"
#include "RunningScripts.h"
#include "common.h"
#include <CRadar.h>
#include <eScriptCommands.h>
#include <cmath>

namespace
{
	constexpr int MISSIONS_REQUIRED = 1;

	constexpr int RIOT_SWEET_LOCATE = 74327 + 184;

	constexpr int RIOT_COUNTER_OFFSET = 2516;
	constexpr int SWEET_CALLED_OFFSET = 5420;
	constexpr int TERRITORY_CALLED_OFFSET = 5416;

	constexpr int RIOT_FINISHED = 5;
	constexpr int END_OF_THE_LINE_STAGE = 2;

	constexpr short LOCATE_PARAM_COUNT = 8;

	const CVector MARKER_POSITION(2488.5f, -1671.0f, 12.6f);
	constexpr float MARKER_RADIUS_SQ = 1.44f;
	constexpr float MARKER_HEIGHT = 2.0f;

	const BranchProgress* g_progress = nullptr;

	Marker marker()
	{
		return { MARKER_POSITION, RADAR_SPRITE_SWEET, Marker::NO_HANDLE_GLOBAL, Marker::LEAVE_DISPLAY };
	}

	bool thresholdMet()
	{
		return g_progress && g_progress->completedMissionCount() >= MISSIONS_REQUIRED;
	}

	bool offerable()
	{
		return thresholdMet() && ScriptGlobals::readAt(RIOT_COUNTER_OFFSET) < RIOT_FINISHED;
	}

	bool playerInMarker()
	{
		CPlayerPed* player = FindPlayerPed();
		if (!player || player->bInVehicle) return false;

		CVector position = player->GetPosition();
		float dx = position.x - MARKER_POSITION.x;
		float dy = position.y - MARKER_POSITION.y;

		return dx * dx + dy * dy <= MARKER_RADIUS_SQ
			&& std::fabs(position.z - MARKER_POSITION.z) <= MARKER_HEIGHT;
	}

	bool takeShortcut(CRunningScript* t_script)
	{
		if (!RunningScripts::isAtInstruction(t_script, RIOT_SWEET_LOCATE)) return false;
		if (!offerable() || !playerInMarker()) return false;

		t_script->CollectParameters(LOCATE_PARAM_COUNT);

		int counterSlot = ScriptGlobals::slotOf(RIOT_COUNTER_OFFSET);
		if (ScriptGlobals::read(counterSlot) < END_OF_THE_LINE_STAGE)
		{
			ScriptGlobals::write(counterSlot, END_OF_THE_LINE_STAGE);
		}
		ScriptGlobals::write(ScriptGlobals::slotOf(SWEET_CALLED_OFFSET), 1);
		ScriptGlobals::write(ScriptGlobals::slotOf(TERRITORY_CALLED_OFFSET), 1);

		t_script->UpdateCompareFlag(true);
		return true;
	}
}

void EndOfTheLine::install(const BranchProgress& t_progress)
{
	g_progress = &t_progress;

	ScriptCommandHook::replaceCommand(COMMAND_LOCATE_CHAR_ON_FOOT_3D, &takeShortcut);
}

void EndOfTheLine::update()
{
	if (offerable())
	{
		marker().raise();
		return;
	}

	if (thresholdMet()) marker().clearAll();
}
