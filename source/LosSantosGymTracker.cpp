#include "LosSantosGymTracker.h"
#include "common.h"
#include "CTheScripts.h"

LosSantosGymTracker::LosSantosGymTracker(int t_submissionID)
	: SubmissionTracker(t_submissionID)
{
}

bool LosSantosGymTracker::isGymScriptActive() const
{
	for (CRunningScript* script = CTheScripts::pActiveScripts; script; script = script->m_pNext)
	{
		if (_strnicmp(script->m_szName, "gymls", 8) == 0) return true;
	}
	return false;
}

void LosSantosGymTracker::enforceSubmissionReward()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return;

	if (isGymScriptActive() && !submissionCompleted)
	{
		if (!m_detectionArmed)
		{
			if (player->m_nFightingStyle == STYLE_BOXING)
			{
				player->m_nFightingStyle = STYLE_STANDARD;
			}
			m_detectionArmed = true;
		}
		return;
	}
	m_detectionArmed = false;

	// m_nFightingStyle also holds weapon-melee styles while certain weapons are equipped
	// (observed live as 15 in the gym), so only ever swap between STANDARD and BOXING and
	// leave every other value alone.
	if (checkReceived && player->m_nFightingStyle == STYLE_STANDARD)
	{
		player->m_nFightingStyle = STYLE_BOXING;
	}
	if (!checkReceived && submissionCompleted && player->m_nFightingStyle == STYLE_BOXING)
	{
		player->m_nFightingStyle = STYLE_STANDARD;
	}
}

bool LosSantosGymTracker::pollCompletion()
{
	if (submissionCompleted || !m_detectionArmed) return false;
	if (!isGymScriptActive()) return false;

	CPlayerPed* player = FindPlayerPed();
	if (!player) return false;

	return player->m_nFightingStyle == STYLE_BOXING;
}
