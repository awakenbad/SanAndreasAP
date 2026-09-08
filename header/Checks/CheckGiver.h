#pragma once
#include <string>

class CheckGiver
{
public:
	CheckGiver() = default;

	void giveMoney(int t_amount);
	void giveWeapon(const std::string& t_weaponType, bool t_equip = false);
	void giveProgressiveMap();

	void giveArmorRefill();

	void giveWeaponMastery(const std::string& t_weaponName);

	void giveMaxSkill(int t_statId);

	void giveCarRepair();

	void update();
private:
	bool m_carRepairPending = false;
};
