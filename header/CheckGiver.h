#pragma once
#include <plugin.h> 
#include "CWorld.h"

class CheckGiver
{
public:
	CheckGiver() = default;
	
	void giveMoney(int t_amount);
	void giveWeapon(std::string t_weaponType);
	void giveProgressiveMission();
	void removeProgressiveMission();
	void giveProgressiveMap();
	int getProgressiveMissionCounter();
private:
	int progressiveMissionCounter = 1;
};

