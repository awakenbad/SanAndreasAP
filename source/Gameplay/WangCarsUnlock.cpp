#include "WangCarsUnlock.h"

#include <CRunningScript.h>
#include <CStats.h>
#include <eScriptCommands.h>

#include "EntityIDs.h"
#include "RunningScripts.h"
#include "ScriptCommandHook.h"
#include "ScriptGlobals.h"

namespace
{
	constexpr int SCRIPT_BASE_OFFSET = 183113;

	constexpr int WANG_CARS_BODY_OFFSET = 1200;
	constexpr int WANG_CARS_BODY_END = 1207;
	constexpr int SHOWROOM_BLIP_OFFSET = 1215;
	constexpr int SHOWROOM_BLIP_END = 1220;

	constexpr int BUY1_BASE_OFFSET = 82993;
	constexpr int LOCKED_PICKUP_OFFSET = 21;
	constexpr int LOCKED_PICKUP_END = 26;

	constexpr int STEAL_BLIP_OFFSET = 57054;
	constexpr int STEAL_BLIP_END = 57071;

	constexpr int STEAL_SCRIPT_OFFSET = 69493;

	constexpr int STEAL_MARKER_X_GLOBAL = 556;

	constexpr float WANG_CARS_PRICE = 50000.0f;
	constexpr float ONE_PROGRESS_POINT = 1.0f;

	bool g_blockVanillaUnlock = false;

	bool blockWangCarsUnlockGate(CRunningScript* t_script)
	{
		if (!g_blockVanillaUnlock) return false;
		if (_strnicmp(t_script->m_szName, "MOB_SF", 8) != 0) return false;

		tScriptParam* variable = t_script->GetPointerToScriptVariable(2);
		return variable == ScriptGlobals::address(WANG_CARS_AVAILABLE_ID);
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
	if (ScriptGlobals::readFloat(STEAL_MARKER_X_GLOBAL) == 0.0f) return;

	unsigned char* scriptSpace = reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace);
	unsigned char* scriptBase = scriptSpace + SCRIPT_BASE_OFFSET;
	unsigned char* buyBase = scriptSpace + BUY1_BASE_OFFSET;

	int savedParams[32];
	memcpy(savedParams, ScriptParams, sizeof(savedParams));

	CRunningScript script;
	script.Init();
	script.m_pBaseIP = scriptSpace;

	RunningScripts::runScript(script, scriptBase + WANG_CARS_BODY_OFFSET, scriptBase + WANG_CARS_BODY_END);
	RunningScripts::runScript(script, scriptBase + SHOWROOM_BLIP_OFFSET, scriptBase + SHOWROOM_BLIP_END);
	RunningScripts::runScript(script, buyBase + LOCKED_PICKUP_OFFSET, buyBase + LOCKED_PICKUP_END);
	RunningScripts::runScript(script, scriptSpace + STEAL_BLIP_OFFSET, scriptSpace + STEAL_BLIP_END);

	CTheScripts::StartNewScript(scriptSpace + STEAL_SCRIPT_OFFSET);

	CStats::IncrementStat(STAT_PROGRESS_MADE, ONE_PROGRESS_POINT);
	CStats::IncrementStat(STAT_PROPERTY_BUDGET, WANG_CARS_PRICE);

	memcpy(ScriptParams, savedParams, sizeof(savedParams));
}
