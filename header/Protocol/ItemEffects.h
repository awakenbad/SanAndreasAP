#pragma once
#include <string>
#include "NotificationOverlay.h"

enum class ItemEffect
{
	Money,
	Weapon,
	ProgressiveMission,
	ProgressiveMap,
	SubmissionCheck, // unlocks one submission's checks; which one is in submissionId
	CollectibleUnlock,
	SubmissionUnlock,
	MaxSkill,
	WeaponMastery,   // maxes one weapon's skill; which one arrives as the value
	ArmorRefill,
	CarRepair,
	StreetRaces,
	WangCars,
	Trap,
};

class ItemEffectSpec
{
public:
	const char* name;
	ItemEffect effect;
	int submissionId;
	const char* trapName;
	const char* message;
	NotificationIcon icon;
};

const ItemEffectSpec* findItemEffect(const std::string& t_name);

std::string formatItemMessage(const ItemEffectSpec& t_spec, const std::string& t_value);
