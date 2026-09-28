#pragma once
#include "RunningScripts.h"
#include <cstddef>

class HeldCheck
{
public:
	int instruction;
	bool (*condition)();
};

template <std::size_t N>
const HeldCheck* heldCheckAt(CRunningScript* t_script, const HeldCheck (&t_checks)[N])
{
	for (const HeldCheck& check : t_checks)
	{
		if (RunningScripts::isAtInstruction(t_script, check.instruction)) return &check;
	}
	return nullptr;
}
