#pragma once
#include "PersistentState.h"

#include <chrono>
#include <string>
#include <vector>

class CVehicle;

class TrapHandler : public PersistentState
{
public:
	void save(SaveDataManager& t_saveData) override;
	void load(const SaveDataManager& t_saveData) override;
	void giveTrap(const std::string& t_trapType);
	void update();
	void drawTimers() const;

private:
	using Clock = std::chrono::steady_clock;
	int randomSeconds(int t_low, int t_high) const;
	static constexpr int TRAP_MIN_SECONDS = 30;
	static constexpr int TRAP_MAX_SECONDS = 120;
	static constexpr int WEATHER_MIN_SECONDS = 60;
	static constexpr int WEATHER_MAX_SECONDS = 180;

	static constexpr int WANTED_TRAP_MIN_STARS = 2;
	int randomWantedStars() const;

	Clock::time_point m_tireTrapEnd{};

	Clock::time_point m_fatTrapEnd{};
	bool m_fatTrapActive = false;
	float m_savedFat = 0.0f;
	float m_savedMuscle = 0.0f;

	bool m_carFirePending = false;

	Clock::time_point m_weatherTrapEnd{};
	bool m_weatherTrapActive = false;
	short m_forcedWeather = 0;

	std::vector<std::string> m_deferredTraps;

	void applyTrap(const std::string& t_trapType);

	void startWeatherTrap(short t_weather);

	void burstTires(CVehicle* t_vehicle);
};
