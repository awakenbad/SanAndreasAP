#include "WangCarsUnlock.h"

#include "ScriptCommandHook.h"
#include "ScriptGlobals.h"
#include <CRunningScript.h>
#include <eScriptCommands.h>

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

	bool g_blockVanillaUnlock = false;

	bool blockWangCarsUnlockGate(CRunningScript* t_script)
	{
		if (!g_blockVanillaUnlock) return false;
		if (_strnicmp(t_script->m_szName, "MOB_SF", 8) != 0) return false;

		tScriptParam* variable = t_script->GetPointerToScriptVariable(2);
		return (int) variable == (int) ScriptGlobals::address(WANG_CARS_AVAILABLE_ID);
	}
}

void WangCarsUnlock::blockVanillaUnlock()
{
	g_blockVanillaUnlock = true;

	ScriptCommandHook::blockCommand(COMMAND_IS_INT_VAR_EQUAL_TO_NUMBER, &blockWangCarsUnlockGate);
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
