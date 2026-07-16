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
	// Call once per tick. Returns true if an existing save was just loaded, meaning callers
	// should re-pull every tracked value back into game state via getValue().
	bool poll();

	void setValue(const std::string& key, const std::string& value);
	std::string getValue(const std::string& key, const std::string& defaultValue) const;

private:
	std::unordered_map<std::string, std::string> m_values;

	// The active companion file is keyed off the real save's own filename (e.g. "GTASAsf1.b"),
	// so each of GTA SA's 8 save slots gets independent AP state. Empty until the player has
	// loaded or saved at least once this process - there is no save to associate data with yet.
	std::string m_currentSaveKey;
	std::string m_lastSeenSaveFileName;
	std::string m_lastSeenLoadFileName;

	std::string getFilePath() const;
	void loadFromDisk();
	void writeToDisk() const;
};
