#pragma once
#include "CTheScripts.h"
#include <cstdint>

namespace ScriptGlobals
{
	inline constexpr int COUNT = 16384;
	inline constexpr uintptr_t SCRIPT_SPACE_OPERAND = 0x463DAE;

	inline unsigned char* scriptSpace()
	{
		return *reinterpret_cast<unsigned char**>(SCRIPT_SPACE_OPERAND);
	}

	inline int read(int t_index)
	{
		if (t_index < 0 || t_index >= COUNT) return 0;
		return *reinterpret_cast<int*>(scriptSpace() + t_index * 4);
	}

	inline float readFloat(int t_index)
	{
		if (t_index < 0 || t_index >= COUNT) return 0.0f;
		return *reinterpret_cast<float*>(scriptSpace() + t_index * 4);
	}

	inline void write(int t_index, int t_value)
	{
		if (t_index < 0 || t_index >= COUNT) return;
		*reinterpret_cast<int*>(scriptSpace() + t_index * 4) = t_value;
	}

	inline void* address(int t_index)
	{
		if (t_index < 0 || t_index >= COUNT) return nullptr;
		return scriptSpace() + t_index * 4;
	}

	inline constexpr int slotOf(int t_byteOffset)
	{
		return t_byteOffset / 4;
	}

	inline int readAt(int t_byteOffset)
	{
		return read(slotOf(t_byteOffset));
	}
}
