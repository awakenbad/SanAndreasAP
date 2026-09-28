#include "CrossBranchUnlocks.h"
#include "ScriptCommandHook.h"
#include "PhoneCall.h"
#include "HeldCheck.h"
#include "ScriptGlobals.h"
#include <eScriptCommands.h>

namespace
{
	constexpr int MOB_LA1 = 180158;
	constexpr int MOB_VEG = 185175;

	constexpr int SCRASH_COUNTER_OFFSET = 2184;
	constexpr int SNAIL_TRAIL_STAGE = 1;
	constexpr int BCRASH_COUNTER_OFFSET = 1972;
	constexpr int BADLANDS_DONE = 1;

	bool always()
	{
		return true;
	}

	bool snailTrailAfterBadlands()
	{
		return ScriptGlobals::readAt(SCRASH_COUNTER_OFFSET) == SNAIL_TRAIL_STAGE
			&& ScriptGlobals::readAt(BCRASH_COUNTER_OFFSET) >= BADLANDS_DONE;
	}

	constexpr HeldCheck REPLACED_CHECKS[] = {
		{ 66412 + 191, &always },
		{ 66700 + 483, &always },
		{ 68612 + 464, &always },
		{ 67844 + 184, &snailTrailAfterBadlands },
		{ 95290 + 680, &snailTrailAfterBadlands },
	};

	constexpr PhoneCall CALLS[] = {
		{ 5384, 1808, 7, { MOB_LA1 + 1643, MOB_LA1 + 1691 } },
		{ 5528, 2388, 8, { MOB_VEG + 934, MOB_VEG + 983 } },
		{ 5512, 2392, 1, { MOB_VEG + 405, MOB_VEG + 440 } },
	};

	constexpr short COMPARE_PARAM_COUNT = 2;

	bool replaceCheck(CRunningScript* t_script)
	{
		const HeldCheck* check = heldCheckAt(t_script, REPLACED_CHECKS);
		if (!check) return false;

		t_script->CollectParameters(COMPARE_PARAM_COUNT);
		t_script->UpdateCompareFlag(check->condition());
		return true;
	}
}

void CrossBranchUnlocks::install()
{
	ScriptCommandHook::replaceCommand(COMMAND_IS_INT_VAR_EQUAL_TO_NUMBER, &replaceCheck);
	ScriptCommandHook::replaceCommand(COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER, &replaceCheck);
}

void CrossBranchUnlocks::update()
{
	for (const PhoneCall& call : CALLS)
	{
		call.runIfDue();
	}
}
