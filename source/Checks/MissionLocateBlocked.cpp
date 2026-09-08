#include "MissionLocateBlocked.h"

#include <CRunningScript.h>
#include <CTheScripts.h>
#include <eScriptCommands.h>

#include "BranchProgress.h"
#include "MissionBranches.h"
#include "ScriptCommandHook.h"

namespace
{
	constexpr short LOCATE_PARAM_COUNT = 8;
	const BranchProgress* g_progress = nullptr;

	float collectedFloat(int t_index)
	{
		return *reinterpret_cast<float*>(&ScriptParams[t_index]);
	}

	bool blockLocate(CRunningScript* t_script)
	{
		if (t_script->m_bIsMission || !g_progress) return false;

		t_script->CollectParameters(LOCATE_PARAM_COUNT);

		int marker = missionMarkerIndexAt(collectedFloat(1), collectedFloat(2));
		return marker >= 0 && markerIsBlocked(marker, *g_progress);
	}
}

void MissionLocateBlocked::install(const BranchProgress& t_progress)
{
	g_progress = &t_progress;

	ScriptCommandHook::blockCommand(COMMAND_LOCATE_CHAR_ANY_MEANS_3D, &blockLocate);
	ScriptCommandHook::blockCommand(COMMAND_LOCATE_CHAR_ON_FOOT_3D, &blockLocate);
	ScriptCommandHook::blockCommand(COMMAND_LOCATE_CHAR_IN_CAR_3D, &blockLocate);
}
