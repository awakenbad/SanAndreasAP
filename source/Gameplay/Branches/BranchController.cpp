#include "BranchController.h"
#include "BranchProgress.h"
#include "ScriptGlobals.h"

namespace
{
	constexpr int GLOBAL_SIZE = 4;
}

CVector BranchController::positionAt(int t_byteOffset, int t_strideBytes)
{
	int slot = ScriptGlobals::slotOf(t_byteOffset);
	int step = t_strideBytes / GLOBAL_SIZE;

	return CVector(ScriptGlobals::readFloat(slot),
		ScriptGlobals::readFloat(slot + step),
		ScriptGlobals::readFloat(slot + step * 2));
}

int BranchController::counter() const
{
	return ScriptGlobals::readAt(m_row.counterOffset);
}

bool BranchController::finished() const
{
	return counter() >= m_row.terminatesAt;
}

bool BranchController::gateOpen(const BranchProgress& t_progress) const
{
	return m_row.requiresMission == NO_PREREQUISITE
		|| t_progress.missionCompleted(m_row.requiresMission);
}

bool BranchController::running() const
{
	unsigned char* entry = reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace) + m_row.address;

	for (CRunningScript* script = CTheScripts::pActiveScripts; script; script = script->m_pNext)
	{
		if (script->m_pCurrentIP == entry) return true;
		if (_strnicmp(script->m_szName, m_row.scriptName, sizeof(script->m_szName)) == 0) return true;
	}
	return false;
}

bool BranchController::positionsInitialised() const
{
	CVector position = positionAt(m_row.positionOffset);
	return position.x != 0.0f || position.y != 0.0f;
}

Marker BranchController::defaultMarker() const
{
	return { positionAt(m_row.positionOffset), ScriptGlobals::readAt(m_row.spriteOffset),
		m_row.blipHandleOffset, m_row.blipDisplay };
}
