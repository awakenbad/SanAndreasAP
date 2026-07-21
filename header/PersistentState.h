#pragma once

class SaveDataManager;

// Implemented by every subsystem holding state the vanilla save format knows nothing about, so
// each one owns its own `_ap.dat` keys instead of Mod knowing all of them.
//
// The reason this is an interface rather than a pair of calls per subsystem: Mod walks ONE list
// in both directions, so a subsystem physically cannot end up saved but never restored (or the
// reverse). Those two halves used to sit ~40 lines apart in Mod, which made adding a field and
// forgetting one side of it the easiest mistake in the file.
class PersistentState
{
public:
	virtual ~PersistentState() = default;

	// Called every tick, so whatever is in memory is already staged when the player next saves.
	virtual void save(SaveDataManager& t_saveData) = 0;

	// Called only once a save has really been loaded - never on a fresh New Game, where
	// restoring another slot's data is the failure mode that must stay impossible.
	virtual void load(const SaveDataManager& t_saveData) = 0;
};
