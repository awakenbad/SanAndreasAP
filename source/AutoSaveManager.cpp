#include "AutoSaveManager.h"
#include "common.h"
#include "CTheScripts.h"
#include <CGenericGameStorage.h>
#include <cstdio>
#include <cstring>
#include <string>

void AutoSaveManager::requestSave()
{
	m_savePending = true;
}

// A mission script is still running through its cleanup for a while after the mission is
// actually passed. Saving in that window is the documented way to produce a save that won't
// load, so wait for every mission script to go away first.
bool AutoSaveManager::isMissionScriptActive() const
{
	for (CRunningScript* script = CTheScripts::pActiveScripts; script; script = script->m_pNext)
	{
		if (script->m_bIsMission && script->m_bIsActive) return true;
	}
	return false;
}

bool AutoSaveManager::update()
{
	if (!m_savePending) return false;
	if (isMissionScriptActive()) return false;
	if (!FindPlayerPed()) return false;

	m_savePending = false;
	return performSave();
}

bool AutoSaveManager::performSave()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return false;

	// Saving while the player is flagged as being in a vehicle is a known save-corruption
	// source, so clear the flag across the write and put it straight back.
	bool wasInVehicle = player->bInVehicle;
	player->bInVehicle = false;

	CGenericGameStorage::MakeValidSaveName(AUTOSAVE_SLOT - 1);
	bool saved = CGenericGameStorage::GenericSave(0);

	player->bInVehicle = wasInVehicle;

	if (saved)
	{
		patchSaveTitle();
	}
	return saved;
}

// The game titles every save with the resolved "last mission passed" text, which would make
// the autosave indistinguishable from a manual save in the load menu. The header stores that
// title as plain text, so rewrite it in place after the save completes. The file ends with a
// 4-byte additive checksum of every preceding byte (the scheme save editors rely on), fixed up
// incrementally from the bytes swapped. If the file doesn't start with the expected "BLOCK"
// tag, it is left untouched - a default title beats a corrupted save.
void AutoSaveManager::patchSaveTitle() const
{
	FILE* file = nullptr;
	if (fopen_s(&file, CGenericGameStorage::ms_SaveFileNameJustSaved, "r+b") != 0 || !file) return;

	char tag[5] = {};
	if (fread(tag, 1, sizeof(tag), file) != sizeof(tag) || memcmp(tag, "BLOCK", 5) != 0)
	{
		fclose(file);
		return;
	}

	char oldTitle[TITLE_SIZE] = {};
	fseek(file, TITLE_OFFSET, SEEK_SET);
	if (fread(oldTitle, 1, TITLE_SIZE, file) != TITLE_SIZE)
	{
		fclose(file);
		return;
	}

	// The game just resolved the mission name itself and wrote it as the title - reuse those
	// bytes rather than doing our own GXT lookup (which proved unreliable here). Strip any
	// existing prefix first so repeated autosaves don't stack "Autosave: Autosave: ...".
	const std::string prefix = "Autosave: ";
	std::string existing(oldTitle, strnlen(oldTitle, TITLE_SIZE));
	if (existing.rfind(prefix, 0) == 0)
	{
		existing = existing.substr(prefix.size());
	}
	std::string title = existing.empty() ? std::string("Autosave") : prefix + existing;

	char newTitle[TITLE_SIZE] = {};
	strncpy_s(newTitle, sizeof(newTitle), title.c_str(), TITLE_SIZE - 1);

	fseek(file, TITLE_OFFSET, SEEK_SET);
	fwrite(newTitle, 1, TITLE_SIZE, file);

	fseek(file, 0, SEEK_END);
	long fileSize = ftell(file);
	unsigned int checksum = 0;
	fseek(file, fileSize - 4, SEEK_SET);
	if (fread(&checksum, sizeof(checksum), 1, file) == 1)
	{
		for (int i = 0; i < TITLE_SIZE; ++i)
		{
			checksum -= static_cast<unsigned char>(oldTitle[i]);
			checksum += static_cast<unsigned char>(newTitle[i]);
		}
		fseek(file, fileSize - 4, SEEK_SET);
		fwrite(&checksum, sizeof(checksum), 1, file);
	}
	fclose(file);
}
