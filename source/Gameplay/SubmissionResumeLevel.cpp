#include "SubmissionResumeLevel.h"

#include <CRunningScript.h>
#include <CStats.h>
#include <algorithm>
#include <cstring>
#include <eScriptCommands.h>

#include "EntityIDs.h"
#include "ScriptCommandHook.h"
#include "ScriptGlobals.h"

namespace
{
	constexpr int PARAMEDIC_LEVEL_LOCAL = 37;
	constexpr int VIGILANTE_LEVEL_LOCAL = 51;
	constexpr int PIMPING_LEVEL_LOCAL = 49;
	constexpr int PARAMEDIC_LEVEL_GLOBAL = 8211;
	constexpr int FIREFIGHTER_LEVEL_GLOBAL = 8213;

	class LevelVariable
	{
	public:
		const char* scriptName;
		bool isLocal;
		int variable;
		int startValue;
		int highestStat;
		int maxLevel;
	};

	constexpr LevelVariable LEVEL_VARIABLES[] = {
		{ "ambulan", true,  PARAMEDIC_LEVEL_LOCAL,  1, STAT_HIGHEST_PARAMEDIC_MISSION_LEVEL, PARAMEDIC_TIERS.tierCount },
		{ "ambulan", false, PARAMEDIC_LEVEL_GLOBAL, 1, STAT_HIGHEST_PARAMEDIC_MISSION_LEVEL, PARAMEDIC_TIERS.tierCount },
		{ "copcar",  true,  VIGILANTE_LEVEL_LOCAL,  0, STAT_HIGHEST_VIGILANTE_MISSION_LEVEL, VIGILANTE_TIERS.tierCount },
		{ "firetru", false, FIREFIGHTER_LEVEL_GLOBAL, 1, STAT_HIGHEST_FIREFIGHTER_MISSION_LEVEL, FIREFIGHTER_TIERS.tierCount },
		{ "pimp",    true,  PIMPING_LEVEL_LOCAL,    0, STAT_PIMPING_LEVEL, PIMPING_TIERS.tierCount },
	};

	constexpr int FIRST_LEVEL = 1;
	constexpr short LEVEL_PARAM_COUNT = 1;

	const void* addressOf(const LevelVariable& t_level)
	{
		if (!t_level.isLocal) return ScriptGlobals::address(t_level.variable);
		return &CTheScripts::LocalVariablesForCurrentMission[t_level.variable];
	}

	int resumeLevel(const LevelVariable& t_level)
	{
		int reached = static_cast<int>(CStats::GetStatValue(t_level.highestStat)) + 1;
		return std::min(std::max(reached, FIRST_LEVEL), t_level.maxLevel);
	}

	bool setStartingLevel(CRunningScript* t_script)
	{
		if (!t_script->m_bIsMission) return false;

		tScriptParam* destination = t_script->GetPointerToScriptVariable(2);
		t_script->CollectParameters(LEVEL_PARAM_COUNT);

		for (const LevelVariable& level : LEVEL_VARIABLES)
		{
			if (ScriptParams[0] != level.startValue) continue;
			if (destination != addressOf(level)) continue;
			if (_strnicmp(t_script->m_szName, level.scriptName, 8) != 0) continue;

			destination->iParam = level.startValue + resumeLevel(level) - FIRST_LEVEL;
			return true;
		}
		return false;
	}
}

void SubmissionResumeLevel::install()
{
	ScriptCommandHook::replaceCommand(COMMAND_SET_VAR_INT, &setStartingLevel);
	ScriptCommandHook::replaceCommand(COMMAND_SET_LVAR_INT, &setStartingLevel);
}
