#include "CrossBranchUnlocks.h"
#include "ScriptCommandHook.h"
#include "PhoneCall.h"
#include "RunningScripts.h"
#include <eScriptCommands.h>

namespace
{
	constexpr int MOB_LA1 = 180158;
	constexpr int MOB_VEG = 185175;

	constexpr int FORCED_CHECKS[] = {
		66412 + 191,
		66700 + 483,
		68612 + 464,
	};

	constexpr PhoneCall CALLS[] = {
		{ 5384, 1808, 7, { MOB_LA1 + 1643, MOB_LA1 + 1691 } },
		{ 5528, 2388, 8, { MOB_VEG + 934, MOB_VEG + 983 } },
		{ 5512, 2392, 1, { MOB_VEG + 405, MOB_VEG + 440 } },
	};

	constexpr short COMPARE_PARAM_COUNT = 2;

	bool forceCheck(CRunningScript* t_script)
	{
		for (int instruction : FORCED_CHECKS)
		{
			if (!RunningScripts::isAtInstruction(t_script, instruction)) continue;

			t_script->CollectParameters(COMPARE_PARAM_COUNT);
			t_script->UpdateCompareFlag(true);
			return true;
		}
		return false;
	}
}

void CrossBranchUnlocks::install()
{
	ScriptCommandHook::replaceCommand(COMMAND_IS_INT_VAR_EQUAL_TO_NUMBER, &forceCheck);
	ScriptCommandHook::replaceCommand(COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER, &forceCheck);
}

void CrossBranchUnlocks::update()
{
	for (const PhoneCall& call : CALLS)
	{
		call.runIfDue();
	}
}
