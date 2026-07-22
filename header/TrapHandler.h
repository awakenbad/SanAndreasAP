#pragma once
#include <chrono>
#include <string>
#include <vector>
#include "PersistentState.h"

class CVehicle;

// Trap items received from AP. Two timed effects (flat tires, fat CJ) and two instant ones
// (wanted level, car fire).
class TrapHandler : public PersistentState
{
public:
	// Only the fat trap has anything to persist - see the members below.
	void save(SaveDataManager& t_saveData) override;
	void load(const SaveDataManager& t_saveData) override;

	// t_trapType is the suffix of the client's GIVE:trap_<type> message. Held until the player is
	// in control - traps mutate CJ and his vehicle, which is exactly what strands a scripted
	// sequence mid-cutscene (see PlayerControl).
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

	// Fat: stats forced to max fat / no muscle for the duration, then restored. Unlike the other
	// traps this one mutates STAT_FAT/STAT_MUSCLE, which the GTA save itself stores - so a save
	// written mid-trap (an autosave fires on every check, so this is easy to hit) used to keep CJ
	// fat with the pre-trap values gone from memory, permanently. Hence save()/load().
	Clock::time_point m_fatTrapEnd{};
	bool m_fatTrapActive = false;
	float m_savedFat = 0.0f;
	float m_savedMuscle = 0.0f;

	// Car fire: applied to the current vehicle, or held until CJ next enters one.
	bool m_carFirePending = false;

	// Traps that arrived while the player wasn't in control, applied in arrival order once they
	// are. Not persisted: quitting mid-cutscene loses a queued trap, which only ever works in the
	// player's favour and matters far less than a softlocked mission.
	std::vector<std::string> m_deferredTraps;

	// Applies a trap with no control check.
	void applyTrap(const std::string& t_trapType);

	// Bursts every tire, whichever kind of vehicle this is. Silently does nothing for the ones
	// with no tires to burst (boats, planes, trains).
	void burstTires(CVehicle* t_vehicle);
};
