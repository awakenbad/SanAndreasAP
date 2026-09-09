#include "HorseBettingHandler.h"

#include "ScriptCommandHook.h"
#include <CHud.h>
#include <CRunningScript.h>
#include <CTheScripts.h>
#include <eScriptCommands.h>
#include "ModSettings.h"

namespace
{
	bool printHorseBettingText(CRunningScript* t_script)
	{
		if (_strnicmp(t_script->m_szName, "OTB_MSC", 8) != 0) return false;

		tScriptParam* destination = t_script->GetPointerToScriptVariable(2);
		if (destination != t_script->GetPointerToLocalVariable(1)) return false;

		t_script->CollectParameters(1);
		if (ScriptParams[0] == 1)
		{
			CHud::SetHelpMessage("The Umamusume servers are currently down for maintenance.", false, true, true);
			return true;
		}

		return false;
	}
}

void HorseBettingHandler::install()
{
	if (!ModSettings::horseBettingDisabled()) return;

	ScriptCommandHook::replaceCommand(COMMAND_SET_LVAR_INT, &printHorseBettingText);
}