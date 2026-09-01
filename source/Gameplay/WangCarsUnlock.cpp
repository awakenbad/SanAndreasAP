#include "WangCarsUnlock.h"
#include "EntityIDs.h"
#include "ScriptGlobals.h"
#include "ScriptCommandHook.h"
#include <CRunningScript.h>
#include <eScriptCommands.h>
#include <cstring>

namespace
{
	constexpr int WANG_CARS_AVAILABLE_ID = 1399;

	constexpr int SCRIPT_BASE_OFFSET = 183113;

	constexpr int WANG_CARS_BODY_OFFSET = 1200;
	constexpr int WANG_CARS_BODY_END = 1243;

	void runScript(CRunningScript& t_script, unsigned char* t_from, unsigned char* t_to)
	{
		t_script.m_pCurrentIP = t_from;
		while (t_script.m_pCurrentIP < t_to)
		{
			t_script.ProcessOneCommand();
		}
	}

	constexpr short COMPARE_PARAM_COUNT = 2;

	bool g_blockVanillaUnlock = false;

	bool blockDesertMissionGate(CRunningScript* t_script)
	{
		if (!g_blockVanillaUnlock) return false;
		if (_strnicmp(t_script->m_szName, "MOB_SF", 8) != 0) return false;

		t_script->CollectParameters(COMPARE_PARAM_COUNT);
		return ScriptParams[0] == WANG_CARS_AVAILABLE_ID;
	}
}

void WangCarsUnlock::blockVanillaUnlock()
{
	g_blockVanillaUnlock = true;

	ScriptCommandHook::blockCommand(COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER, &blockDesertMissionGate);
}

void WangCarsUnlock::update(bool t_itemReceived)
{
	if (!t_itemReceived) return;
	if (ScriptGlobals::read(WANG_CARS_AVAILABLE_ID) == 1) return;

	unsigned char* scriptBase = reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace) + SCRIPT_BASE_OFFSET;

	int savedParams[32];
	memcpy(savedParams, ScriptParams, sizeof(savedParams));

	CRunningScript script;
	script.Init();
	script.m_pBaseIP = reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace);

	runScript(script, scriptBase + WANG_CARS_BODY_OFFSET, scriptBase + WANG_CARS_BODY_END);

	memcpy(ScriptParams, savedParams, sizeof(savedParams));
}
