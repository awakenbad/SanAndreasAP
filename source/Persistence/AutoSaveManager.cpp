#include "AutoSaveManager.h"

#include <CGenericGameStorage.h>
#include <CTheScripts.h>
#include <common.h>
#include <cstdio>
#include <cstring>
#include <string>

#include "GameStorageHook.h"
#include "SaveRedirect.h"

void AutoSaveManager::requestSave()
{
	m_savePending = true;
}

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

	bool wasInVehicle = player->bInVehicle;
	player->bInVehicle = false;

	GameStorageHook::notifyBeforeSave();

	std::string path = SaveRedirect::saveFileName(AUTOSAVE_SLOT - 1);
	strncpy_s(CGenericGameStorage::ms_ValidSaveName, VALID_SAVE_NAME_SIZE, path.c_str(), _TRUNCATE);
	bool saved = CGenericGameStorage::GenericSave(0);

	player->bInVehicle = wasInVehicle;

	if (saved)
	{
		patchSaveTitle();
	}
	return saved;
}

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
