#pragma once
#include <string>

class SaveDataManager;

// Autosaves the game after story progress, so a crash costs one mission instead of a whole
// session. The AP companion file rides along automatically: SaveDataManager::poll() sees the
// resulting save-name change on the next tick and writes <savename>_ap.dat to match.
//
// Target slot: if the player has saved manually this session we keep writing to that same save
// file, so autosaves stay where they expect them. Otherwise we use a dedicated slot, so we
// never clobber a save the player didn't choose.
class AutoSaveManager
{
public:
	// Call when meaningful progress completes. The save itself is deferred until the game is in
	// a safe state - saving while a mission script is still running can corrupt the file.
	void requestSave();

	// Call once per tick. Returns true on the tick a save was actually written, so the caller
	// can notify the player.
	bool update(const SaveDataManager& t_saveData);

	// Human-readable description of the slot last written to, for the notification.
	const std::string& getLastSavedSlotName() const { return m_lastSavedSlotName; }

private:
	// 1-based, matching the slot numbering players see. Rainbomizer defaults to 8 as well.
	static constexpr int DEFAULT_SLOT = 8;

	bool isMissionScriptActive() const;
	bool performSave(const SaveDataManager& t_saveData);

	bool m_savePending = false;
	std::string m_lastSavedSlotName;
};
