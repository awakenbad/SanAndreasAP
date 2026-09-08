#include "GymTracker.h"

#include <common.h>

#include "RunningScripts.h"

GymTracker::GymTracker(int t_submissionID, eFightingStyle t_taughtStyle, const char* t_scriptName,
	FightingStyleArbiter& t_arbiter)
	: SubmissionTracker(t_submissionID), m_taughtStyle(t_taughtStyle), m_scriptName(t_scriptName)
	, m_arbiter(t_arbiter)
{
}

namespace
{
	bool isOverwritableStyle(eFightingStyle t_style)
	{
		return t_style == STYLE_STANDARD || t_style == STYLE_BOXING
			|| t_style == STYLE_KUNG_FU || t_style == STYLE_KNEE_HEAD;
	}
}

bool GymTracker::isGymScriptActive() const
{
	return RunningScripts::isActive(m_scriptName);
}

void GymTracker::enforceSubmissionReward()
{
	CPlayerPed* player = FindPlayerPed();
	if (!player) return;

	if (isGymScriptActive() && !submissionCompleted)
	{
		if (!m_detectionArmed)
		{
			if (player->m_nFightingStyle == m_taughtStyle)
			{
				player->m_nFightingStyle = STYLE_GRAB_KICK;
			}
			m_detectionArmed = true;
		}
		return;
	}
	m_detectionArmed = false;

	if (!checkReceived)
	{
		m_receiptOrder = 0;
	}
	else if (m_receiptOrder == 0)
	{
		m_receiptOrder = m_arbiter.claim();
	}

	bool ownsStyleSlot = checkReceived && m_receiptOrder == m_arbiter.latest();
	if (ownsStyleSlot
		&& player->m_nFightingStyle != m_taughtStyle
		&& isOverwritableStyle(player->m_nFightingStyle))
	{
		player->m_nFightingStyle = m_taughtStyle;
	}

	if (!checkReceived && submissionCompleted && player->m_nFightingStyle == m_taughtStyle)
	{
		player->m_nFightingStyle = STYLE_STANDARD;
	}
}

bool GymTracker::pollCompletion()
{
	if (submissionCompleted || !m_detectionArmed) return false;
	if (!isGymScriptActive()) return false;

	CPlayerPed* player = FindPlayerPed();
	if (!player) return false;

	return player->m_nFightingStyle == m_taughtStyle;
}
