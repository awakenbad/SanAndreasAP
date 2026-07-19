#pragma once

// Autosaves the game after story progress, so a crash costs one mission instead of a whole
// session. Always writes to its own dedicated slot - the player's manual saves are never
// touched - and retitles the save "Autosave: <mission>" so it is recognizable in the load menu.
// The AP companion file rides along automatically: SaveDataManager::poll() sees the resulting
// save-name change on the next tick and writes <savename>_ap.dat to match.
class AutoSaveManager
{
public:
	// Call when meaningful progress completes. The save itself is deferred until the game is in
	// a safe state - saving while a mission script is still running can corrupt the file.
	void requestSave();

	// Call once per tick. Returns true on the tick a save was actually written, so the caller
	// can notify the player.
	bool update();

private:
	// 1-based, matching the slot numbering players see. Rainbomizer defaults to 8 as well.
	static constexpr int AUTOSAVE_SLOT = 8;

	// The save header's name field: 100 bytes of plain text at the start of the SimpleVars
	// block, preceded in the file by the 5-byte "BLOCK" tag and the 4-byte version id.
	static constexpr long TITLE_OFFSET = 9;
	static constexpr int TITLE_SIZE = 100;

	bool isMissionScriptActive() const;
	bool performSave();
	void patchSaveTitle() const;

	bool m_savePending = false;
};
