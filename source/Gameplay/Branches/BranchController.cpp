#include "BranchController.h"
#include "BranchProgress.h"
#include "ScriptGlobals.h"
#include "RunningScripts.h"

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
	if (m_row.previousCounterOffset == FIRST_IN_BRANCH) return t_progress.received(m_row.branch) > 0;

	return ScriptGlobals::readAt(m_row.previousCounterOffset) >= m_row.previousFinishedAt;
}

bool BranchController::running() const
{
	return RunningScripts::isRunningOrStarting(m_row.scriptName, m_row.address);
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
