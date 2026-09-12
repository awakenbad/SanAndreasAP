#pragma once
#include "PersistentState.h"
#include <vector>

class ChaosModHandler : public PersistentState
{
public:
	enum EffectSeverity
	{
		LOW,
		MEDIUM,
		HIGH
	};

	struct Effect
	{
		EffectSeverity severity;
		const char* effectID;
		const char* displayName;
		int duration = -1;
	};

	void save(SaveDataManager& t_saveData) override;
	void load(const SaveDataManager& t_saveData) override;
	void giveEffect(const EffectSeverity t_effectSeverity);
	void update();

private:
	int randomSeconds(int t_low, int t_high) const;
	static constexpr int TRAP_MIN_SECONDS = 30;
	static constexpr int TRAP_MAX_SECONDS = 120;

	std::vector<const Effect*> m_deferredEffects;

	const Effect* getRandomEffect(const EffectSeverity t_maxEffectSeverity);

	void applyEffect(const Effect* t_trapType);
};
