#pragma once
#include <plugin.h> 
#include "CWorld.h"

class CheckGiver
{
public:
	CheckGiver() = default;
	
	void giveMoney(int t_amount);
	void giveWeapon(const std::string& t_weaponType, bool t_equip = false);
	void giveProgressiveMission();
	void removeProgressiveMission();
	void giveProgressiveMap();
	int getProgressiveMissionCounter();
	void setProgressiveMissionCounter(int t_value);
private:
	int progressiveMissionCounter = 1;
};

