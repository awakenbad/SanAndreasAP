#include "SaveRedirect.h"

#include <CFileMgr.h>
#include <CGenericGameStorage.h>
#include <Patch.h>
#include <ShlObj_core.h>
#include <windows.h>

#pragma comment(lib, "Shell32.lib")

namespace
{
	constexpr uintptr_t MAKE_VALID_SAVE_NAME_CALL_SITE = 0x619066; // in the pause menu's save
	constexpr uintptr_t MAKE_VALID_SAVE_NAME = 0x5D0E90;
	constexpr uintptr_t SLOT_LISTING_OPEN_CALL_SITE = 0x6191D1;
	constexpr uintptr_t FILE_MGR_OPEN_FILE = 0x538900;
	constexpr uintptr_t LOAD_COMMIT_CALL_SITE = 0x5D13A4; // in CheckSlotDataValid
	constexpr uintptr_t CHECK_DATA_NOT_CORRUPT = 0x5D1170;
	constexpr uintptr_t CHECK_SLOT_DATA_VALID_CALL_SITE = 0x578EF2;
	constexpr uintptr_t CHECK_SLOT_DATA_VALID = 0x5D1380;

	constexpr size_t VALID_SAVE_NAME_SIZE = 256;

	constexpr size_t MAX_WORLD_ID = 16;
	constexpr size_t LOAD_PATH_LIMIT = 104;
	constexpr size_t LONGEST_SAVE_FILE_NAME = 10;

	std::string g_worldId;
	std::string g_directory;

	std::string userFilesDirectory()
	{
		char documentsPath[MAX_PATH] = {};
		SHGetFolderPathA(nullptr, CSIDL_PERSONAL, nullptr, 0, documentsPath);
		return std::string(documentsPath) + "\\GTA San Andreas User Files\\";
	}

	std::string formatWorldId(const std::string& t_worldId)
	{
		std::string result;
		for (char c : t_worldId)
		{
			bool allowed = (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')
				|| (c >= '0' && c <= '9') || c == '_' || c == '-';
			result += allowed ? c : '_';
			if (result.size() >= MAX_WORLD_ID) break;
		}
		return result;
	}

	std::string pathToWorldFolder(const char* t_path)
	{
		std::string path = t_path ? t_path : "";
		size_t pos = path.find_last_of("\\/");
		return SaveRedirect::directory() + (pos == std::string::npos ? path : path.substr(pos + 1));
	}

	void __cdecl onMakeValidSaveName(int t_slot)
	{
		CGenericGameStorage::MakeValidSaveName(t_slot);
		if (!SaveRedirect::isActive()) return;

		std::string path = SaveRedirect::saveFileName(t_slot);
		strncpy_s(CGenericGameStorage::ms_ValidSaveName, VALID_SAVE_NAME_SIZE, path.c_str(), _TRUNCATE);
	}

	int __cdecl onSlotListingOpenFile(char* t_path, const char* t_mode)
	{
		if (SaveRedirect::isActive() && t_path)
		{
			std::string path = pathToWorldFolder(t_path);
			if (path.size() < LOAD_PATH_LIMIT) memcpy(t_path, path.c_str(), path.size() + 1);
		}
		return CFileMgr::OpenFile(t_path, t_mode);
	}

	bool __cdecl onCheckDataNotCorrupt(int t_saveId, char* t_fileName)
	{
		if (!SaveRedirect::isActive() || !t_fileName)
		{
			return CGenericGameStorage::CheckDataNotCorrupt(t_saveId, t_fileName);
		}

		std::string path = pathToWorldFolder(t_fileName);

		if (path.size() >= LOAD_PATH_LIMIT)
		{
			return CGenericGameStorage::CheckDataNotCorrupt(t_saveId, t_fileName);
		}

		memcpy(t_fileName, path.c_str(), path.size() + 1);
		return CGenericGameStorage::CheckDataNotCorrupt(t_saveId, t_fileName);
	}

	bool __cdecl onCheckSlotDataValid(int t_saveId, bool t_unused)
	{
		bool valid = CGenericGameStorage::CheckSlotDataValid(t_saveId, t_unused);
		if (!SaveRedirect::isActive()) return valid;

		std::string path = pathToWorldFolder(CGenericGameStorage::ms_LoadFileName);
		if (path.size() >= LOAD_PATH_LIMIT) return valid;

		strncpy_s(CGenericGameStorage::ms_LoadFileName, LOAD_PATH_LIMIT, path.c_str(), _TRUNCATE);
		strncpy_s(CGenericGameStorage::ms_LoadFileNameWithPath, LOAD_PATH_LIMIT, path.c_str(), _TRUNCATE);
		return valid;
	}

	void patchCallSite(uintptr_t t_site, uintptr_t t_expected, void* t_replacement)
	{
		const unsigned char* site = reinterpret_cast<const unsigned char*>(t_site);
		if (site[0] != 0xE8) return;

		uintptr_t target = t_site + 5 + *reinterpret_cast<const int*>(site + 1);
		if (target != t_expected) return;

		plugin::patch::RedirectCall(t_site, t_replacement);
	}
}

void SaveRedirect::install()
{
	patchCallSite(MAKE_VALID_SAVE_NAME_CALL_SITE, MAKE_VALID_SAVE_NAME, &onMakeValidSaveName);
	patchCallSite(SLOT_LISTING_OPEN_CALL_SITE, FILE_MGR_OPEN_FILE, &onSlotListingOpenFile);
	patchCallSite(LOAD_COMMIT_CALL_SITE, CHECK_DATA_NOT_CORRUPT, &onCheckDataNotCorrupt);
	patchCallSite(CHECK_SLOT_DATA_VALID_CALL_SITE, CHECK_SLOT_DATA_VALID, &onCheckSlotDataValid);
}

void SaveRedirect::setWorld(const std::string& t_worldId)
{
	std::string worldId = formatWorldId(t_worldId);
	if (worldId.empty() || worldId == g_worldId) return;

	std::string parent = userFilesDirectory() + "archipelago";
	CreateDirectoryA(parent.c_str(), nullptr);

	std::string directory = parent + "\\" + worldId + "\\";
	if (directory.size() + LONGEST_SAVE_FILE_NAME >= LOAD_PATH_LIMIT) return;

	CreateDirectoryA(directory.c_str(), nullptr);
	if (GetFileAttributesA(directory.c_str()) == INVALID_FILE_ATTRIBUTES) return;

	g_worldId = worldId;
	g_directory = directory;
}

bool SaveRedirect::isActive()
{
	return !g_directory.empty();
}

const std::string& SaveRedirect::directory()
{
	if (g_directory.empty())
	{
		static const std::string vanilla = userFilesDirectory();
		return vanilla;
	}
	return g_directory;
}

std::string SaveRedirect::saveFileName(int t_slot)
{
	return directory() + "GTASAsf" + std::to_string(t_slot + 1) + ".b";
}
