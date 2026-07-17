#pragma once
#include <chrono>
#include <string>

// Trap items received from AP. Two timed effects (flat tires, fat CJ) and two instant ones
// (wanted level, car fire).
class TrapHandler
{
public:
	// t_trapType is the suffix of the client's GIVE:trap_<type> message.
	void giveTrap(const std::string& t_trapType);

	// Call once per tick.
	void update();

	// Persistent countdown lines for active timed traps, drawn top-right under the native HUD
	// stats. Call every frame from the HUD draw event.
	void drawTimers() const;

private:
	using Clock = std::chrono::steady_clock;

	int randomDurationSeconds() const;

	// Flat tires: while the timer runs, every automobile CJ occupies gets all tires burst.
	Clock::time_point m_tireTrapEnd{};

	// Fat: stats forced to max fat / no muscle for the duration, then restored. Not persisted -
	// restarting the game mid-trap keeps the fat stats (accepted limitation).
	Clock::time_point m_fatTrapEnd{};
	bool m_fatTrapActive = false;
	float m_savedFat = 0.0f;
	float m_savedMuscle = 0.0f;

	// Car fire: applied to the current vehicle, or held until CJ next enters one.
	bool m_carFirePending = false;
};
