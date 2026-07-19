#pragma once
#include <string>
#include <unordered_map>

// Persists arbitrary AP mod state (progressive mission counter, submission reward flags, ...)
// to a companion file next to the actual GTA SA save it belongs to, since none of that state
// is part of the vanilla save format and would otherwise reset to defaults every time the game
// process restarts. Values are held as strings so callers don't need a shared schema here.
class SaveDataManager
{
public:
	// Call once per tick: detects completed game saves and writes the companion file through.
	// Loads are deliberately NOT detected here - ms_LoadFileName changes while merely browsing
	// the load menu, so restores are driven by the caller's world-wipe signal instead.
	void poll();

	// Restore path, called when the caller has established a save really was just loaded (the
	// blip-pool wipe signal plus a non-fresh-game check): adopts whatever ms_LoadFileName
	// currently holds as the active save and restores its companion file.
	bool restoreFromCurrentLoadName();

	const std::string& getCurrentSaveKey() const;

	// True once a real save has completed this session (player-initiated or autosave), meaning
	// getCurrentSaveKey() names a file the player is actively using.
	bool hasSavedThisSession() const { return m_hasSavedThisSession; }

	void setValue(const std::string& key, const std::string& value);
	std::string getValue(const std::string& key, const std::string& defaultValue) const;

private:
	std::unordered_map<std::string, std::string> m_values;

	// The active companion file is keyed off the real save's own filename (e.g. "GTASAsf1.b"),
	// so each of GTA SA's 8 save slots gets independent AP state. Empty until the player has
	// loaded or saved at least once this process - there is no save to associate data with yet.
	std::string m_currentSaveKey;
	std::string m_lastSeenSaveFileName;

	bool m_initialized = false;
	bool m_hasSavedThisSession = false;

	std::string getFilePath() const;
	void loadFromDisk();
	void writeToDisk() const;
};
