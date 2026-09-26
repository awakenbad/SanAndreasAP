#pragma once
#include "ScriptSlice.h"

class PhoneCall
{
public:
	int calledOffset;
	int counterOffset;
	int counterAtLeast;
	ScriptSlice body;

	void runIfDue() const;
};
