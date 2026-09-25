#include "Marker.h"
#include <CRadar.h>
#include <extensions/ScriptCommands.h>
#include "ScriptGlobals.h"

namespace
{
	constexpr bool FOREIGN_SPRITES_ONLY = false;
	constexpr bool EVERY_SPRITE = true;
	constexpr float TOLERANCE_SQ = 9.0f;
}

bool Marker::stillDrawn() const
{
	int handle = ScriptGlobals::readAt(blipHandleOffset);
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