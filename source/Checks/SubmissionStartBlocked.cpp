#include "SubmissionStartBlocked.h"
#include "Submissions/SubmissionTracker.h"
#include "ScriptCommandHook.h"
#include "EntityIDs.h"
#include "ScriptGlobals.h"
#include <CRunningScript.h>
#include <CTheScripts.h>
#include <eScriptCommands.h>
#include "common.h"
#include <CHud.h>

namespace
{
	const std::vector<std::unique_ptr<SubmissionTracker>>* g_trackers = nullptr;

	constexpr int ODDVEH_BASE = 81837;
	constexpr int ODDVEH_COURIER_LAUNCH_START = 270;
	constexpr int ODDVEH_COURIER_END = 465;

	bool isCourierTrigger(CRunningScript* t_script)
	{
		if (_strnicmp(t_script->m_szName, "oddveh", 8) != 0) return false;

		unsigned char* base = ScriptGlobals::scriptSpace() + ODDVEH_BASE;
		auto offset = t_script->m_pCurrentIP - base;
		return offset >= ODDVEH_COURIER_LAUNCH_START && offset < ODDVEH_COURIER_END;
	}

	void showLockedMessage()
	{
		CHud::SetHelpMessage("You have not unlocked this submission yet.", false, false, false);
		CHud::m_nHelpMessageTimer = 5; // Force set it to 5 seconds in case we show it while another help message is already shown
	}

	void refuseCourierLaunch()
	{
		if (ScriptGlobals::read(COURIER_TRIGGER_GLOBAL) != 0) return;
		if (CHud::HelpMessageDisplayed()) return;

		showLockedMessage();
		ScriptGlobals::write(COURIER_TRIGGER_GLOBAL, 1);
	}

bool preventSubmissionStart(CRunningScript* t_script)
{
	bool courier = isCourierTrigger(t_script);
	if (_strnicmp(t_script->m_szName, "R3", 10) != 0 && !courier) return false;
	if (!g_trackers) return false;

	CPlayerPed* player = FindPlayerPed();
	if (!player) return false;
	if (!player->bInVehicle || !player->m_pVehicle) return false;

	int vehicleModelId = player->m_pVehicle->m_nModelIndex;

	for (const auto& tracker : *g_trackers)
	{
		if (!tracker->isVehicleValid(vehicleModelId)) continue;
		if (tracker->isUnlocked()) break;

		if (courier) refuseCourierLaunch();

		t_script->UpdateCompareFlag(false);
		return true;
	}

	return false;
}
}

void SubmissionStartBlocked::update(const std::vector<std::unique_ptr<SubmissionTracker>>& t_trackers)
{
	g_trackers = &t_trackers;

	ScriptCommandHook::blockCommand(COMMAND_IS_CHAR_IN_MODEL, &preventSubmissionStart);
	ScriptCommandHook::blockCommand(COMMAND_IS_CHAR_IN_TAXI, &preventSubmissionStart);
	ScriptCommandHook::blockCommand(COMMAND_IS_CHAR_IN_ANY_POLICE_VEHICLE, &preventSubmissionStart);

	keyHandler();
}

void SubmissionStartBlocked::keyHandler()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return;
	if (!player->bInVehicle || !player->m_pVehicle) return;

	auto pad = CPad::GetPad(0);
	if (!pad || !pad->GetLookBehindForPed()) return;

	if (CHud::HelpMessageDisplayed()) return;

	int vehicleModelId = player->m_pVehicle->m_nModelIndex;

	for (const auto& tracker : *g_trackers)
	{
		if (!tracker->isVehicleValid(vehicleModelId)) continue;

		// Submission unlocked
		if (tracker->isUnlocked()) return;

		showLockedMessage();
	}
}