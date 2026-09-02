#include "StreetRaceUnlock.h"
#include "EntityIDs.h"
#include "ScriptGlobals.h"
#include "ScriptCommandHook.h"
#include "RunningScripts.h"
#include <CRunningScript.h>
#include <eScriptCommands.h>
#include <cstring>

namespace
{
	constexpr int SCRIPT_BASE_OFFSET = 183113;

	constexpr int SCHOOL_BODY_OFFSET = 757;
	constexpr int DSCHOOL_BLIP_OFFSET = 772;
	constexpr int DSCHOOL_BLIP_END = 795;
	constexpr int SCHOOL_BODY_END = 867;

	constexpr int RACE_BODY_OFFSET = 988;
	constexpr int RACE_BODY_END = 1079;

	constexpr short COMPARE_PARAM_COUNT = 2;
	constexpr int DESERT_MISSIONS_REQUIRED = 3;

	bool g_blockVanillaUnlock = false;

	bool blockDesertMissionGate(CRunningScript* t_script)
	{
		if (!g_blockVanillaUnlock) return false;
		if (_strnicmp(t_script->m_szName, "MOB_SF", 8) != 0) return false;

		t_script->CollectParameters(COMPARE_PARAM_COUNT);
		return ScriptParams[1] == DESERT_MISSIONS_REQUIRED;
	}
}

void StreetRaceUnlock::blockVanillaUnlock()
{
	g_blockVanillaUnlock = true;

	ScriptCommandHook::blockCommand(COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER, &blockDesertMissionGate);
}

void StreetRaceUnlock::update(bool t_itemReceived)
{
	if (!t_itemReceived) return;
	if (ScriptGlobals::read(STREET_RACES_UNLOCKED_GLOBAL) != 0) return;
	if (ScriptGlobals::read(TRACE_MARKER_X_GLOBAL) == 0) return;

	unsigned char* scriptBase = reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace) + SCRIPT_BASE_OFFSET;

	int savedParams[32];
	memcpy(savedParams, ScriptParams, sizeof(savedParams));

	CRunningScript script;
	script.Init();
	script.m_pBaseIP = reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace);

	if (ScriptGlobals::read(DRIVING_SCHOOL_UNLOCKED_GLOBAL) == 0)
	{
		RunningScripts::runScript(script, scriptBase + SCHOOL_BODY_OFFSET, scriptBase + DSCHOOL_BLIP_OFFSET);
		RunningScripts::runScript(script, scriptBase + DSCHOOL_BLIP_END, scriptBase + SCHOOL_BODY_END);
	}
	RunningScripts::runScript(script, scriptBase + RACE_BODY_OFFSET, scriptBase + RACE_BODY_END);

	memcpy(ScriptParams, savedParams, sizeof(savedParams));
}

void StreetRaceUnlock::updateDrivingSchoolBlip()
{
	if (ScriptGlobals::read(DRIVING_SCHOOL_BLIP_GLOBAL) != 0) return;
	if (ScriptGlobals::read(DRIVING_SCHOOL_UNLOCKED_GLOBAL) == 0) return;
	if (ScriptGlobals::read(GARAGE_MISSIONS_GLOBAL) == 0) return;

	unsigned char* scriptSpace = reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace);

	int savedParams[32];
	memcpy(savedParams, ScriptParams, sizeof(savedParams));

	CRunningScript script;
	script.Init();
	script.m_pBaseIP = scriptSpace;
	RunningScripts::runScript(script, scriptSpace + SCRIPT_BASE_OFFSET + DSCHOOL_BLIP_OFFSET,
		scriptSpace + SCRIPT_BASE_OFFSET + DSCHOOL_BLIP_END);

	memcpy(ScriptParams, savedParams, sizeof(savedParams));
}
