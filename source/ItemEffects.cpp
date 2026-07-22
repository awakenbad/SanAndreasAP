#include "ItemEffects.h"
#include "EntityIDs.h"

namespace
{
	// The single source of truth for every item the mod understands.
	constexpr ItemEffectSpec ITEM_EFFECTS[] = {
		{ "money",               ItemEffect::Money,              -1,                  nullptr,     "Archipelago: Received ${}",                 NotificationIcon::Money },
		{ "weapon",              ItemEffect::Weapon,             -1,                  nullptr,     "Archipelago: Received weapon ({})",         NotificationIcon::Weapon },
		{ "progressive_mission", ItemEffect::ProgressiveMission, -1,                  nullptr,     "Archipelago: Received a Progressive Mission", NotificationIcon::ProgressiveMission },
		{ "progressive_map",     ItemEffect::ProgressiveMap,     -1,                  nullptr,     "Archipelago: Received a Progressive Map",   NotificationIcon::None },

		{ "health_upgrade",      ItemEffect::SubmissionCheck,    PARAMEDIC_ID,        nullptr,     "Archipelago: Received Max Health Upgrade",  NotificationIcon::HealthUpgrade },
		{ "armor_upgrade",       ItemEffect::SubmissionCheck,    VIGILANTE_ID,        nullptr,     "Archipelago: Received Max Armor Upgrade",   NotificationIcon::ArmorUpgrade },
		{ "fire_immunity",       ItemEffect::SubmissionCheck,    FIREFIGHTER_ID,      nullptr,     "Archipelago: Received Fire Immunity",       NotificationIcon::FireImmunity },
		{ "stamina_upgrade",     ItemEffect::SubmissionCheck,    BURGLARY_ID,         nullptr,     "Archipelago: Received Infinite Sprint",     NotificationIcon::Stamina },
		{ "taxi_nitro",          ItemEffect::SubmissionCheck,    TAXI_ID,             nullptr,     "Archipelago: Received Taxi Nitro",          NotificationIcon::Taxi },
		{ "boxing_style",        ItemEffect::SubmissionCheck,    LOS_SANTOS_GYM_ID,   nullptr,     "Archipelago: Received Boxing Style",        NotificationIcon::Boxing },

		{ "armor_refill",        ItemEffect::ArmorRefill,        -1,                  nullptr,     "Archipelago: Received Full Armor",          NotificationIcon::ArmorUpgrade },
		{ "car_repair",          ItemEffect::CarRepair,          -1,                  nullptr,     "Archipelago: Received Car Repair",          NotificationIcon::Taxi },

		{ "trap_tires",          ItemEffect::Trap,               -1,                  "tires",     "Archipelago: Flat Tires Trap!",             NotificationIcon::Trap },
		{ "trap_fat",            ItemEffect::Trap,               -1,                  "fat",       "Archipelago: Fat CJ Trap!",                 NotificationIcon::Trap },
		{ "trap_wanted",         ItemEffect::Trap,               -1,                  "wanted",    "Archipelago: Wanted Level Trap!",           NotificationIcon::Trap },
		{ "trap_carfire",        ItemEffect::Trap,               -1,                  "carfire",   "Archipelago: Car Fire Trap!",               NotificationIcon::Trap },
	};

	constexpr char VALUE_PLACEHOLDER[] = "{}";
}

const ItemEffectSpec* findItemEffect(const std::string& t_name)
{
	// A linear scan over ~18 rows, a handful of times per session - a map would cost more to set
	// up than it could ever save here.
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
