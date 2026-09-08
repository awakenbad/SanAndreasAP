#include "StartingSaves.h"

#include <filesystem>
#include <windows.h>

#include "MenuGate.h"
#include "SaveRedirect.h"

namespace
{
	constexpr char LOS_SANTOS[] = "los_santos";
	constexpr char SEEDED_SLOT[] = "GTASAsf1.b";
	constexpr char COMPANION_SUFFIX[] = "_ap.dat";
	constexpr size_t MAX_NAME = 32;

	std::string g_startingPoint;
	std::string g_missingSaveName;

	bool isPlainName(const std::string& t_name)
	{
		if (t_name.empty() || t_name.size() > MAX_NAME) return false;
		for (char c : t_name)
		{
			if (!((c >= 'a' && c <= 'z') || c == '_')) return false;
		}
		return true;
	}

	std::string shippedSavesDirectory()
	{
		HMODULE module = nullptr;
		if (!GetModuleHandleExA(
			GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
			reinterpret_cast<LPCSTR>(&shippedSavesDirectory), &module))
		{
			return {};
		}

		char path[MAX_PATH]{};
		if (GetModuleFileNameA(module, path, MAX_PATH) == 0) return {};

		return (std::filesystem::path(path).parent_path() / "Archipelago").string() + "\\";
	}

	bool worldFolderHasSaves()
	{
		std::error_code error;
		std::filesystem::directory_iterator entries(SaveRedirect::directory(), error);
		if (error) return true;

		for (const std::filesystem::directory_entry& entry : entries)
		{
			if (entry.path().extension() == ".b") return true;
		}
		return false;
	}
}

void StartingSaves::setStartingPoint(const std::string& t_name)
{
	g_startingPoint = isPlainName(t_name) ? t_name : std::string();
}

void StartingSaves::seedIfNeeded()
{
	g_missingSaveName.clear();

	if (!SaveRedirect::isActive()) return;
	if (g_startingPoint.empty() || g_startingPoint == LOS_SANTOS) return;
	if (worldFolderHasSaves()) return;

	std::string saveName = g_startingPoint + ".b";
	std::string source = shippedSavesDirectory() + saveName;
	std::string destination = SaveRedirect::directory() + SEEDED_SLOT;

	if (!CopyFileA(source.c_str(), destination.c_str(), TRUE))
	{
		g_missingSaveName = saveName;
		return;
	}
	CopyFileA((source + COMPANION_SUFFIX).c_str(),
		(destination + COMPANION_SUFFIX).c_str(), TRUE);

	MenuGate::refreshSaveSlotList();
}

const std::string& StartingSaves::missingSaveName()
{
	return g_missingSaveName;
}
