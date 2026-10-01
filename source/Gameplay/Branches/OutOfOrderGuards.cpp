#include "OutOfOrderGuards.h"
#include "ScriptCommandHook.h"
#include "ScriptGlobals.h"
#include "RunningScripts.h"
#include <CStats.h>
#include <Patch.h>
#include <eScriptCommands.h>
#include <eStats.h>

namespace
{
	constexpr int FLOW_DISPATCHED_OFFSET = 232;

	constexpr int SAVE_ICON_COUNT_OFFSET = 3540;
	constexpr int SAVE_PICKUPS_CREATED_OFFSET = 3536;
	constexpr int ALL_SAVE_ICONS = 18;

	constexpr int MOB_LA1_CITIES_CHECK = 180158 + 60;
	constexpr int GREEN_SABRE_COUNTER_OFFSET = 1832;
	constexpr int GREEN_SABRE_FINISHED = 2;

	constexpr int OPENUP_LAST_CITY_CHECK = 99724 + 1577;
	constexpr int OPENUP_CITY_LATCHES[] = { 680, 684, 688, 692 };

	constexpr uintptr_t NO_RECRUITS_WHILE_CITIES_PASSED_1_OR_2 = 0x60C972;
	constexpr size_t XOR_SIZE = 2;

	bool keepCitiesPassedFromDropping(CRunningScript* t_script)
	{
		t_script->CollectParameters(2);
		if (ScriptParams[0] != STAT_CITY_UNLOCKED) return false;

		int value = ScriptParams[1];
		return value > 0 && value < static_cast<int>(CStats::GetStatValue(STAT_CITY_UNLOCKED));
	}

	bool holdMobLa1UntilGreenSabre(CRunningScript* t_script)
	{
		if (!RunningScripts::isAtInstruction(t_script, MOB_LA1_CITIES_CHECK)) return false;
		return ScriptGlobals::readAt(GREEN_SABRE_COUNTER_OFFSET) < GREEN_SABRE_FINISHED;
	}

	bool holdOpenUpUntilEveryCityOpened(CRunningScript* t_script)
	{
		if (!RunningScripts::isAtInstruction(t_script, OPENUP_LAST_CITY_CHECK)) return false;

		for (int latch : OPENUP_CITY_LATCHES)
		{
			if (ScriptGlobals::readAt(latch) == 0) return true;
		}
		return false;
	}

	void keepFlowRunning()
	{
		int flowSlot = ScriptGlobals::slotOf(FLOW_DISPATCHED_OFFSET);
		if (ScriptGlobals::read(flowSlot) == 0) ScriptGlobals::write(flowSlot, 1);
	}

	void unlockEverySaveIcon()
	{
		int countSlot = ScriptGlobals::slotOf(SAVE_ICON_COUNT_OFFSET);
		if (ScriptGlobals::read(countSlot) >= ALL_SAVE_ICONS) return;

		ScriptGlobals::write(countSlot, ALL_SAVE_ICONS);
		ScriptGlobals::write(ScriptGlobals::slotOf(SAVE_PICKUPS_CREATED_OFFSET), 0);
	}
}

void OutOfOrderGuards::install()
{
	ScriptCommandHook::replaceCommand(COMMAND_SET_INT_STAT, &keepCitiesPassedFromDropping);
	ScriptCommandHook::blockCommand(COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER, &holdMobLa1UntilGreenSabre);
	ScriptCommandHook::blockCommand(COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER, &holdOpenUpUntilEveryCityOpened);
	plugin::patch::Nop(NO_RECRUITS_WHILE_CITIES_PASSED_1_OR_2, XOR_SIZE);
}

void OutOfOrderGuards::update()
{
	keepFlowRunning();
	unlockEverySaveIcon();
}
