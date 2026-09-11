#include "ChaosModHandler.h"
#include "PlayerControl.h"
#include "SaveDataManager.h"
#include <format>
#include <random>
#include <vector>
#include <windows.h>

using QueueEffectFn = void (*)(const char*);

namespace
{
	constexpr ChaosModHandler::Effect effects[] =
	{
		{ ChaosModHandler::LOW, "effect_dvd_screensaver", "DVD Screensaver" },
		{ ChaosModHandler::LOW, "effect_one_bullet_magazines", "One Bullet Magazines" },
		{ ChaosModHandler::LOW, "effect_pedal_to_the_metal", "Pedal To The Metal" },
		{ ChaosModHandler::LOW, "effect_shaky_hands", "Shaky Hands" },
		{ ChaosModHandler::LOW, "effect_struck_by_truck", "You've been struck by..." },

		{ ChaosModHandler::MEDIUM, "effect_half_game_speed", "0.5x Game Speed" },
		{ ChaosModHandler::MEDIUM, "effect_double_game_speed", "2x Game Speed" },
		{ ChaosModHandler::MEDIUM, "effect_random_teleport", "Random Teleport" },

		{ ChaosModHandler::MEDIUM, "effect_beyblade", "Beyblade" },
		{ ChaosModHandler::MEDIUM, "effect_bumper_peds", "Bumper Peds" },
		{ ChaosModHandler::MEDIUM, "effect_one_percent_death", "Death (1% Chance)" },
		{ ChaosModHandler::MEDIUM, "effect_cops_everywhere", "Cops Everywhere" },

		{ ChaosModHandler::MEDIUM, "effect_freefall", "Freefall!" },
		{ ChaosModHandler::MEDIUM, "effect_inverted_controls", "Inverted Controls" },
		{ ChaosModHandler::MEDIUM, "effect_remove_all_weapons", "Remove All Weapons" },
		{ ChaosModHandler::MEDIUM, "effect_send_vehicles_to_space", "Send Vehicles To Space" },
		{ ChaosModHandler::MEDIUM, "effect_vehicle_one_hit_ko", "Vehicle One Hit K.O." },

		{ ChaosModHandler::HIGH, "effect_death", "Death" },
		{ ChaosModHandler::HIGH, "effect_wanted_level_six_stars", "Six Wanted Stars" },
		{ ChaosModHandler::HIGH, "effect_explode_all_cars", "Explode All Vehicles" },
		{ ChaosModHandler::HIGH, "effect_carmageddon", "Carmageddon" },
		{ ChaosModHandler::HIGH, "effect_insane_gravity", "Insane Gravity", 10000 },
	};

	int randomBetween(int t_low, int t_high)
	{
		static std::mt19937 generator{ std::random_device{}() };
		return std::uniform_int_distribution<int>(t_low, t_high)(generator);
	}
}

int ChaosModHandler::randomSeconds(int t_low, int t_high) const
{
	return randomBetween(t_low, t_high);
}

void ChaosModHandler::save(SaveDataManager& t_saveData)
{

}

void ChaosModHandler::load(const SaveDataManager& t_saveData)
{

}

const ChaosModHandler::Effect* ChaosModHandler::getRandomEffect(const EffectSeverity t_maxEffectSeverity)
{
	std::vector<const ChaosModHandler::Effect*> eligibleEffects;

	for (const auto& effect : effects)
	{
		if (effect.severity <= t_maxEffectSeverity)
		{
			eligibleEffects.push_back(&effect);
		}
	}

	if (eligibleEffects.empty()) return nullptr;

	static std::mt19937 generator{ std::random_device{}() };
	std::uniform_int_distribution<size_t> dist(0, eligibleEffects.size() - 1);

	return eligibleEffects[dist(generator)];
}

void ChaosModHandler::giveEffect(const EffectSeverity t_effectSeverity)
{
	auto randomEffect = getRandomEffect(t_effectSeverity);

	if (!PlayerControl::isInControl())
	{
		m_deferredEffects.push_back(randomEffect);
		return;
	}

	applyEffect(randomEffect);
}

void ChaosModHandler::applyEffect(const Effect* t_effect)
{
	HMODULE hMod = GetModuleHandleA("trilogychaosmod.sa.asi");
	if (!hMod) return; // Chaos mod is not installed

	auto QueueEffect = (QueueEffectFn) GetProcAddress(hMod, "QueueEffect");

	if (QueueEffect)
	{
		int duration = (t_effect->duration != -1)
			? t_effect->duration
			: randomSeconds(TRAP_MIN_SECONDS, TRAP_MAX_SECONDS) * 1000;

		std::string jsonStr = std::format(
			R"({{"effectID":"{}","displayName":"{}","duration":{},"subtext":"Archipelago","drawnTemporarily":true}})",
			t_effect->effectID,
			t_effect->displayName,
			duration
		);

		QueueEffect(jsonStr.c_str());
	}
}

void ChaosModHandler::update()
{
	if (!m_deferredEffects.empty() && PlayerControl::isInControl())
	{
		std::vector<const Effect*> toApply;
		toApply.swap(m_deferredEffects);
		for (const Effect* effect : toApply)
		{
			applyEffect(effect);
		}
	}
}
