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

	// Refills armor to the player's current maximum (respects the Max Armor Upgrade).
	void giveArmorRefill();

	// Fully repairs the current vehicle, or the next one CJ enters if on foot.
	void giveCarRepair();

	// Call once per tick - applies a pending car repair once CJ is in a vehicle.
	void update();
private:
	int progressiveMissionCounter = 1;
	bool m_carRepairPending = false;
};

