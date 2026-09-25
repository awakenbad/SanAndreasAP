#pragma once
// Everything shared between branches
class BranchRow
{
public:
	const char* scriptName;
	int address;
	int counterOffset;
	int terminatesAt;
	int positionOffset;
	int spriteOffset;
	int blipHandleOffset;
	int blipDisplay;
	int requiresMission;
};