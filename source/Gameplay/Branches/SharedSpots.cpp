#include "SharedSpots.h"
#include "ScriptCommandHook.h"
#include "ScriptGlobals.h"
#include "HeldCheck.h"
#include <eScriptCommands.h>

namespace
{
	constexpr int INT_COUNTER_OFFSET = 1792;
	constexpr int SWEET_COUNTER_OFFSET = 1808;
	constexpr int GREEN_SABRE_COUNTER_OFFSET = 1832;
	constexpr int CASINO_COUNTER_OFFSET = 2388;
	constexpr int MANSION_COUNTER_OFFSET = 2504;
	constexpr int GROVE_COUNTER_OFFSET = 2508;
	constexpr int RIOT_COUNTER_OFFSET = 2516;

	constexpr int SWEET_FINISHED = 9;
	constexpr int GREEN_SABRE_FINISHED = 2;
	constexpr int FISH_IN_A_BARREL_DONE = 7;
	constexpr int MANSION_FINISHED = 4;
	constexpr int GROVE_FINISHED = 2;
	constexpr int END_OF_THE_LINE_STAGE = 2;

	class FinishedCounter
	{
	public:
		int offset;
		int finishedAt;
	};

	constexpr FinishedCounter LOS_SANTOS_BRANCHES[] = {
		{ SWEET_COUNTER_OFFSET, SWEET_FINISHED },
		{ 1812, 3 },
		{ 1816, 4 },
		{ 1820, 5 },
		{ 1824, 2 },
		{ 1828, 1 },
	};

	bool losSantosFinished()
	{
		for (const FinishedCounter& branch : LOS_SANTOS_BRANCHES)
		{
			if (ScriptGlobals::readAt(branch.offset) < branch.finishedAt) return false;
		}
		return true;
	}

	bool sweetsHouseTaken()
	{
		if (ScriptGlobals::readAt(SWEET_COUNTER_OFFSET) < SWEET_FINISHED) return true;
		return losSantosFinished() && ScriptGlobals::readAt(GREEN_SABRE_COUNTER_OFFSET) < GREEN_SABRE_FINISHED;
	}

	bool fourDragonsTaken()
	{
		return ScriptGlobals::readAt(CASINO_COUNTER_OFFSET) < FISH_IN_A_BARREL_DONE;
	}

	bool johnsonHouseTaken()
	{
		return ScriptGlobals::readAt(INT_COUNTER_OFFSET) == 0;
	}

	bool mansionTaken()
	{
		return ScriptGlobals::readAt(MANSION_COUNTER_OFFSET) < MANSION_FINISHED;
	}

	bool riotAtSweetsHouseHeld()
	{
		return sweetsHouseTaken()
			|| ScriptGlobals::readAt(GROVE_COUNTER_OFFSET) < GROVE_FINISHED
			|| ScriptGlobals::readAt(RIOT_COUNTER_OFFSET) >= END_OF_THE_LINE_STAGE;
	}

	constexpr HeldCheck HELD_LOCATES[] = {
		{ 73661 + 90, &fourDragonsTaken },
		{ 74046 + 90, &johnsonHouseTaken },
		{ 74046 + 184, &sweetsHouseTaken },
		{ 74327 + 74, &mansionTaken },
		{ 74327 + 184, &riotAtSweetsHouseHeld },
	};

	bool holdLaterScript(CRunningScript* t_script)
	{
		const HeldCheck* locate = heldCheckAt(t_script, HELD_LOCATES);
		return locate && locate->condition();
	}
}

void SharedSpots::install()
{
	ScriptCommandHook::blockCommand(COMMAND_LOCATE_CHAR_ON_FOOT_3D, &holdLaterScript);
}
