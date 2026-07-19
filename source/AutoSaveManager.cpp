#include "AutoSaveManager.h"
#include "SaveDataManager.h"
#include "common.h"
#include "CTheScripts.h"
#include <CGenericGameStorage.h>
#include <cstdlib>

// GTA SA names its saves "GTASAsf<N>.b" for slots 1-8, while MakeValidSaveName takes the
// 0-based index - so the file GTASAsf3.b is MakeValidSaveName(2). Returns -1 if the name
// doesn't look like a save slot we can target.
static int slotFromSaveFileName(const std::string& t_fileName)
{
	std::string digits;
	for (char c : t_fileName)
	{
		if (c >= '0' && c <= '9') digits += c;
		else if (!digits.empty()) break;
	}
	if (digits.empty()) return -1;

	int slot = std::atoi(digits.c_str());
	return (slot >= 1 && slot <= 8) ? slot : -1;
}

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

bool AutoSaveManager::update(const SaveDataManager& t_saveData)
{
	if (!m_savePending) return false;
	if (isMissionScriptActive()) return false;
	if (!FindPlayerPed()) return false;

	m_savePending = false;
	return performSave(t_saveData);
}

bool AutoSaveManager::performSave(const SaveDataManager& t_saveData)
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return false;

	int slot = DEFAULT_SLOT;
	if (t_saveData.hasSavedThisSession())
	{
		int currentSlot = slotFromSaveFileName(t_saveData.getCurrentSaveKey());
		if (currentSlot > 0) slot = currentSlot;
	}

	// Saving while the player is flagged as being in a vehicle is a known save-corruption
	// source, so clear the flag across the write and put it straight back. 
	// bInVehicle is bit 0x100 of CPed's flag block - the exact bit they mask.
	bool wasInVehicle = player->bInVehicle;
	player->bInVehicle = false;

	CGenericGameStorage::MakeValidSaveName(slot - 1);
	bool saved = CGenericGameStorage::GenericSave(0);

	player->bInVehicle = wasInVehicle;

	if (saved)
	{
		m_lastSavedSlotName = "slot " + std::to_string(slot);
	}
	return saved;
}
