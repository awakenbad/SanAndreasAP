#include "SaveDataManager.h"
#include <CGenericGameStorage.h>
#include <fstream>
#include <shlobj.h>

#pragma comment(lib, "Shell32.lib")

static std::string extractFileName(const std::string& pathOrName)
{
	size_t pos = pathOrName.find_last_of("\\/");
	return pos == std::string::npos ? pathOrName : pathOrName.substr(pos + 1);
}

bool SaveDataManager::poll()
{
	bool restoreNeeded = false;

	std::string loadName = extractFileName(CGenericGameStorage::ms_LoadFileName ? CGenericGameStorage::ms_LoadFileName : "");
	std::string saveName = extractFileName(CGenericGameStorage::ms_SaveFileNameJustSaved ? CGenericGameStorage::ms_SaveFileNameJustSaved : "");

	if (!m_initialized)
	{
		m_lastSeenLoadFileName = loadName;
		m_lastSeenSaveFileName = saveName;
		m_initialized = true;
		return false;
	}

	if (!loadName.empty() && loadName != m_lastSeenLoadFileName)
	{
		m_lastSeenLoadFileName = loadName;
		m_currentSaveKey = loadName;
		m_lastSeenSaveFileName = loadName;
		loadFromDisk();
		restoreNeeded = true;
	}

	if (!saveName.empty() && saveName != m_lastSeenSaveFileName)
	{
		m_lastSeenSaveFileName = saveName;
		m_currentSaveKey = saveName;
		writeToDisk();
	}

	return restoreNeeded;
}

void SaveDataManager::setValue(const std::string& key, const std::string& value)
{
	auto it = m_values.find(key);
	if (it != m_values.end() && it->second == value) return;

	m_values[key] = value;
	if (!m_currentSaveKey.empty())
	{
		writeToDisk();
	}
}

std::string SaveDataManager::getValue(const std::string& key, const std::string& defaultValue) const
{
	auto it = m_values.find(key);
	return it != m_values.end() ? it->second : defaultValue;
}

std::string SaveDataManager::getFilePath() const
{
	char documentsPath[MAX_PATH] = {};
	SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, documentsPath);
	return std::string(documentsPath) + "\\GTA San Andreas User Files\\" + m_currentSaveKey + "_ap.dat";
}

void SaveDataManager::loadFromDisk()
{
	m_values.clear();

	std::ifstream file(getFilePath());
	if (!file.is_open()) return;

	std::string line;
	while (std::getline(file, line))
	{
		size_t eq = line.find('=');
		if (eq == std::string::npos) continue;
		m_values[line.substr(0, eq)] = line.substr(eq + 1);
	}
}

void SaveDataManager::writeToDisk() const
{
	std::ofstream file(getFilePath(), std::ios::trunc);
	if (!file.is_open()) return;

	for (const auto& [key, value] : m_values)
	{
		file << key << "=" << value << "\n";
	}
}
