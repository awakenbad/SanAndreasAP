#pragma once
#include <CTheScripts.h>

// The SCM's global variable array - where state with no CStats entry lives (driving school scores).
namespace ScriptGlobals
{
	// One 4-byte slot each, at the start of ScriptSpace.
	inline constexpr int COUNT = 16384;

	inline int read(int t_index)
	{
		if (t_index < 0 || t_index >= COUNT) return 0;
		return *reinterpret_cast<int*>(CTheScripts::ScriptSpace + t_index * 4);
	}

	inline float readFloat(int t_index)
	{
		if (t_index < 0 || t_index >= COUNT) return 0.0f;
		return *reinterpret_cast<float*>(CTheScripts::ScriptSpace + t_index * 4);
	}

	inline void write(int t_index, int t_value)
	{
		if (t_index < 0 || t_index >= COUNT) return;
		*reinterpret_cast<int*>(CTheScripts::ScriptSpace + t_index * 4) = t_value;
	}

	inline void* address(int t_index)
	{
		if (t_index < 0 || t_index >= COUNT) return nullptr;
		return CTheScripts::ScriptSpace + t_index * 4;
	}

	inline constexpr int slotOf(int t_byteOffset)
	{
		return t_byteOffset / 4;
	}
}
