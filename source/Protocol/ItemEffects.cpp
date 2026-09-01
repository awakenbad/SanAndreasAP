#include "ItemEffects.h"
#include "EntityIDs.h"
#include <CStats.h>

namespace
{
	constexpr ItemEffectSpec ITEM_EFFECTS[] = {
		{ "money",               ItemEffect::Money,              -1,                  nullptr,     "Archipelago: Received ${}",                 NotificationIcon::Money },
		{ "weapon",              ItemEffect::Weapon,             -1,                  nullptr,     "Archipelago: Received weapon ({})",         NotificationIcon::Weapon },
		{ "progressive_mission", ItemEffect::ProgressiveMission, -1,                  nullptr,     "Archipelago: Received {} Mission",          NotificationIcon::ProgressiveMission },
		{ "progressive_map",     ItemEffect::ProgressiveMap,     -1,                  nullptr,     "Archipelago: Received a Progressive Map",   NotificationIcon::None },

		{ "health_upgrade",      ItemEffect::SubmissionCheck,    PARAMEDIC_ID,        nullptr,     "Archipelago: Received Max Health Upgrade",  NotificationIcon::HealthUpgrade },
		{ "armor_upgrade",       ItemEffect::SubmissionCheck,    VIGILANTE_ID,        nullptr,     "Archipelago: Received Max Armor Upgrade",   NotificationIcon::ArmorUpgrade },
		{ "fire_immunity",       ItemEffect::SubmissionCheck,    FIREFIGHTER_ID,      nullptr,     "Archipelago: Received Fire Immunity",       NotificationIcon::FireImmunity },
		{ "stamina_upgrade",     ItemEffect::SubmissionCheck,    BURGLARY_ID,         nullptr,     "Archipelago: Received Infinite Sprint",     NotificationIcon::Stamina },
		{ "taxi_nitro",          ItemEffect::SubmissionCheck,    TAXI_ID,             nullptr,     "Archipelago: Received Taxi Nitro",          NotificationIcon::Taxi },
		{ "boxing_style",        ItemEffect::SubmissionCheck,    LOS_SANTOS_GYM_ID,   nullptr,     "Archipelago: Received Boxing Style",        NotificationIcon::Boxing },
		{ "kung_fu_style",       ItemEffect::SubmissionCheck,    SAN_FIERRO_GYM_ID,   nullptr,     "Archipelago: Received Kung Fu Style",       NotificationIcon::Boxing },
		{ "kickboxing_style",    ItemEffect::SubmissionCheck,    LAS_VENTURAS_GYM_ID, nullptr,     "Archipelago: Received Kickboxing Style",    NotificationIcon::Boxing },

		{ "max_cycling_skill",   ItemEffect::MaxSkill,           STAT_CYCLING_SKILL,  nullptr,     "Archipelago: Received Max Cycling Skill",   NotificationIcon::Stamina },
		{ "max_driving_skill",   ItemEffect::SubmissionCheck,    DRIVING_SCHOOL_ID,   nullptr,     "Archipelago: Received Max Driving Skill",   NotificationIcon::Taxi },
		{ "max_bike_skill",      ItemEffect::SubmissionCheck,    BIKE_SCHOOL_ID,      nullptr,     "Archipelago: Received Max Bike Skill",      NotificationIcon::Stamina },
		{ "max_muscle",          ItemEffect::MaxSkill,           STAT_MUSCLE,         nullptr,     "Archipelago: Received Max Muscle",          NotificationIcon::Boxing },

		{ "armor_refill",        ItemEffect::ArmorRefill,        -1,                  nullptr,     "Archipelago: Received Full Armor",          NotificationIcon::ArmorUpgrade },
		{ "car_repair",          ItemEffect::CarRepair,          -1,                  nullptr,     "Archipelago: Received Car Repair",          NotificationIcon::Taxi },
		{ "weapon_mastery",      ItemEffect::WeaponMastery,      -1,                  nullptr,     "Archipelago: Received {} Mastery",          NotificationIcon::Weapon },
		{ "street_races",        ItemEffect::StreetRaces,        -1,                  nullptr,       "Archipelago: Unlocked Street Races",         NotificationIcon::StreetRaces },
		{ "wang_cars",			 ItemEffect::WangCars,           -1,                  nullptr,       "Archipelago: Unlocked Wang Cars",            NotificationIcon::WangCars },
		{ "unlock_tags",           ItemEffect::CollectibleUnlock,  -1,                  "TAG",       "Archipelago: Unlocked Tags",                 NotificationIcon::Spray },
		{ "unlock_oysters",        ItemEffect::CollectibleUnlock,  -1,                  "OYSTER",    "Archipelago: Unlocked Oysters",              NotificationIcon::Oyster },
		{ "unlock_horseshoes",     ItemEffect::CollectibleUnlock,  -1,                  "HORSESHOE", "Archipelago: Unlocked Horseshoes",           NotificationIcon::Horseshoe },
		{ "unlock_snapshots",      ItemEffect::CollectibleUnlock,  -1,                  "SNAPSHOT",  "Archipelago: Unlocked Snapshots",            NotificationIcon::Snapshot },
		{ "unlock_paramedic",       ItemEffect::SubmissionUnlock,   PARAMEDIC_ID,        nullptr,     "Archipelago: Unlocked Paramedic",            NotificationIcon::Hospital },
		{ "unlock_firefighter",     ItemEffect::SubmissionUnlock,   FIREFIGHTER_ID,      nullptr,     "Archipelago: Unlocked Firefighter",          NotificationIcon::FireImmunity },
		{ "unlock_vigilante",       ItemEffect::SubmissionUnlock,   VIGILANTE_ID,        nullptr,     "Archipelago: Unlocked Vigilante",            NotificationIcon::ArmorUpgrade },
		{ "unlock_taxi",            ItemEffect::SubmissionUnlock,   TAXI_ID,             nullptr,     "Archipelago: Unlocked Taxi",                 NotificationIcon::Taxi },
		{ "unlock_pimping",         ItemEffect::SubmissionUnlock,   PIMPING_ID,          nullptr,     "Archipelago: Unlocked Pimping",              NotificationIcon::Stamina },
		{ "unlock_burglary",        ItemEffect::SubmissionUnlock,   BURGLARY_ID,         nullptr,     "Archipelago: Unlocked Burglary",             NotificationIcon::Stamina },

		{ "trap_tires",          ItemEffect::Trap,               -1,                  "tires",     "Archipelago: Flat Tires Trap!",             NotificationIcon::Trap },
		{ "trap_fat",            ItemEffect::Trap,               -1,                  "fat",       "Archipelago: Fat CJ Trap!",                 NotificationIcon::Trap },
		{ "trap_wanted",         ItemEffect::Trap,               -1,                  "wanted",    "Archipelago: Wanted Level Trap!",           NotificationIcon::Trap },
		{ "trap_carfire",        ItemEffect::Trap,               -1,                  "carfire",   "Archipelago: Car Fire Trap!",               NotificationIcon::Trap },
		{ "trap_weather",        ItemEffect::Trap,               -1,                  "weather",   "Archipelago: Bad Weather Trap!",            NotificationIcon::Trap },
	};

	constexpr char VALUE_PLACEHOLDER[] = "{}";
}

const ItemEffectSpec* findItemEffect(const std::string& t_name)
{
	for (const ItemEffectSpec& spec : ITEM_EFFECTS)
	{
		if (t_name == spec.name) return &spec;
	}
	return nullptr;
}

std::string formatItemMessage(const ItemEffectSpec& t_spec, const std::string& t_value)
{
	if (!t_spec.message) return "";

	std::string message = t_spec.message;
	size_t placeholder = message.find(VALUE_PLACEHOLDER);
	if (placeholder != std::string::npos)
	{
		message.replace(placeholder, sizeof(VALUE_PLACEHOLDER) - 1, t_value);
	}
	return message;
}
