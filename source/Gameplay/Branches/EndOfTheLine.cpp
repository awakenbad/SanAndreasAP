#include "EndOfTheLine.h"
#include "Marker.h"
#include "BranchProgress.h"
#include "ScriptGlobals.h"
#include "ScriptCommandHook.h"
#include "BranchControllers.h"
#include "RunningScripts.h"
#include "common.h"
#include <CRadar.h>
#include <eScriptCommands.h>
#include <cmath>

namespace
{
	int g_missionsRequired = 0;

	constexpr int RIOT_ADDRESS = 74327;
	constexpr int RIOT_SWEET_LOCATE = RIOT_ADDRESS + 184;

	constexpr int RIOT_COUNTER_OFFSET = 2516;
	constexpr int SWEET_CALLED_OFFSET = 5420;
	constexpr int TERRITORY_CALLED_OFFSET = 5416;

	constexpr int RIOT_FINISHED = 5;
	constexpr int END_OF_THE_LINE_STAGE = 2;

	constexpr short LOCATE_PARAM_COUNT = 8;

	const CVector MARKER_POSITION(2488.5f, -1671.0f, 12.6f);
	constexpr float MARKER_RADIUS_SQ = 1.44f;
	constexpr float MARKER_HEIGHT = 2.0f;
	constexpr float BLIP_TOLERANCE_SQ = 9.0f;

	const BranchProgress* g_progress = nullptr;

	Marker marker()
	{
		return { MARKER_POSITION, RADAR_SPRITE_SWEET, Marker::NO_HANDLE_GLOBAL, Marker::LEAVE_DISPLAY };
	}

	bool configured()
	{
		return g_progress && g_missionsRequired > 0;
	}

	bool riotFinished()
	{
		return ScriptGlobals::readAt(RIOT_COUNTER_OFFSET) >= RIOT_FINISHED;
	}

	bool offerable()
	{
		return EndOfTheLine::unlocked() && !riotFinished();
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

	void startRiotIfNeeded()
	{
		if (RunningScripts::isRunningOrStarting("RIOT", RIOT_ADDRESS)) return;

		CTheScripts::StartNewScript(reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace) + RIOT_ADDRESS);
	}

	bool takeShortcut(CRunningScript* t_script)
	{
		if (!BranchControllers::enabled()) return false;

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

void EndOfTheLine::setMissionsRequired(int t_count)
{
	g_missionsRequired = t_count;
}

void EndOfTheLine::update()
{
	if (!configured()) return;

	if (riotFinished())
	{
		marker().clearAll();
		return;
	}

	marker().raise();
	if (unlocked()) startRiotIfNeeded();
}

bool EndOfTheLine::markerAt(const CVector& t_position)
{
	if (!BranchControllers::enabled() || !configured()) return false;

	float dx = t_position.x - MARKER_POSITION.x;
	float dy = t_position.y - MARKER_POSITION.y;
	return dx * dx + dy * dy <= BLIP_TOLERANCE_SQ;
}

int EndOfTheLine::missionsCompleted()
{
	return g_progress ? g_progress->completedMissionCount() : 0;
}

bool EndOfTheLine::unlocked()
{
	return configured() && missionsCompleted() >= g_missionsRequired;
}
