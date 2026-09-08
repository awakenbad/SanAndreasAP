#include "BranchController.h"

#include <CRadar.h>
#include <eScriptCommands.h>
#include <extensions/ScriptCommands.h>

#include "BranchProgress.h"
#include "ScriptGlobals.h"

namespace
{
	constexpr int GLOBAL_SIZE = 4;
	constexpr float TOLERANCE_SQ = 9.0f;

	constexpr bool FOREIGN_SPRITES_ONLY = false;
	constexpr bool EVERY_SPRITE = true;
}

bool Marker::stillDrawn() const
{
	int handle = ScriptGlobals::read(ScriptGlobals::slotOf(blipHandleOffset));
	if (handle == 0) return false;
	if (!handleMayBeStale) return true;

	int index = CRadar::GetActualBlipArrayIndex(handle);
	return index >= 0 && CRadar::ms_RadarTrace[index].m_bInUse;
}

void Marker::raise() const
{
	if (stillDrawn()) return;
	if (sprite == 0) return;
	if (position.x == 0.0f && position.y == 0.0f && position.z == 0.0f) return;

	int handle = 0;
	plugin::Command<eScriptCommands::COMMAND_ADD_SPRITE_BLIP_FOR_CONTACT_POINT>(
		position.x, position.y, position.z, sprite, &handle);

	ScriptGlobals::write(ScriptGlobals::slotOf(blipHandleOffset), handle);

	if (blipDisplay != LEAVE_DISPLAY)
	{
		plugin::Command<eScriptCommands::COMMAND_CHANGE_BLIP_DISPLAY>(handle, blipDisplay);
	}
}

void Marker::clearForeign() const
{
	clear(FOREIGN_SPRITES_ONLY);
}

void Marker::clearAll() const
{
	clear(EVERY_SPRITE);
}

void Marker::clear(bool t_includeOurSprite) const
{
	int handleSlot = ScriptGlobals::slotOf(blipHandleOffset);

	for (unsigned int i = 0; i < MAX_RADAR_TRACES; ++i)
	{
		const tRadarTrace& trace = CRadar::ms_RadarTrace[i];
		if (!trace.m_bInUse) continue;
		if (trace.m_nBlipType != BLIP_CONTACTPOINT) continue;
		if (!t_includeOurSprite && trace.m_nRadarSprite == sprite) continue;

		float dx = position.x - trace.m_vecPos.x;
		float dy = position.y - trace.m_vecPos.y;
		if (dx * dx + dy * dy > TOLERANCE_SQ) continue;

		int blip = CRadar::GetNewUniqueBlipIndex(static_cast<int>(i));
		if (blip == ScriptGlobals::read(handleSlot)) ScriptGlobals::write(handleSlot, 0);
		CRadar::ClearBlip(blip);
	}
}

int BranchController::readGlobal(int t_byteOffset)
{
	return ScriptGlobals::read(ScriptGlobals::slotOf(t_byteOffset));
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
	return readGlobal(m_row.counterOffset);
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

Marker BranchController::defaultMarker() const
{
	return { positionAt(m_row.positionOffset), readGlobal(m_row.spriteOffset),
		m_row.blipHandleOffset, m_row.blipDisplay };
}
