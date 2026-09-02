#pragma once
#include "CTheScripts.h"
#include <cstring>

namespace RunningScripts
{
	constexpr int OPCODE_SIZE = 2;

	inline bool isActive(const char* t_scriptName)
	{
		for (CRunningScript* script = CTheScripts::pActiveScripts; script; script = script->m_pNext)
		{
			if (_strnicmp(script->m_szName, t_scriptName, 8) == 0) return true;
		}
		return false;
	}

	inline bool isAtInstruction(CRunningScript* t_script, int t_offset)
	{
		unsigned char* instruction = reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace)
			+ t_offset + OPCODE_SIZE;

		return t_script->m_pCurrentIP == instruction;
	}

	inline void runScript(CRunningScript& t_script, unsigned char* t_from, unsigned char* t_to)
	{
		t_script.m_pCurrentIP = t_from;
		while (t_script.m_pCurrentIP < t_to)
		{
			t_script.ProcessOneCommand();
		}
	}
}
