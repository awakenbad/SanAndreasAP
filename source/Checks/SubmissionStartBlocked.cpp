#include "SubmissionStartBlocked.h"

#include <CHud.h>
#include <CRunningScript.h>
#include <common.h>
#include <eScriptCommands.h>

#include "ScriptCommandHook.h"
#include "Submissions/SubmissionTracker.h"

namespace
{
	const std::vector<std::unique_ptr<SubmissionTracker>>* g_trackers = nullptr;

	bool preventSubmissionStart(CRunningScript* t_script)
	{
		if (_strnicmp(t_script->m_szName, "R3", 10) != 0) return false;
		if (!g_trackers) return false;

		CPlayerPed* player = FindPlayerPed();
		if (!player) return false;
		if (!player->bInVehicle || !player->m_pVehicle) return false;

		int vehicleModelId = player->m_pVehicle->m_nModelIndex;

		for (const auto& tracker : *g_trackers)
		{
			if (!tracker->isVehicleValid(vehicleModelId)) continue;
			if (tracker->isUnlocked()) break;

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

		CHud::SetHelpMessage("You have not unlocked this submission yet.", false, false, false);
		CHud::m_nHelpMessageTimer = 5; // Force set it to 5 seconds in case we show it while another help message is already shown
	}
}