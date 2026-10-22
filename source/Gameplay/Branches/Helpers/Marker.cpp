#include "Marker.h"
#include <CRadar.h>
#include <extensions/ScriptCommands.h>
#include "ScriptGlobals.h"

namespace
{
	constexpr bool FOREIGN_SPRITES_ONLY = false;
	constexpr bool EVERY_SPRITE = true;
	constexpr float TOLERANCE = 3.0f;
}

bool Marker::isDrawnHere() const
{
	for (unsigned int i = 0; i < MAX_RADAR_TRACES; ++i)
	{
		const tRadarTrace& trace = CRadar::ms_RadarTrace[i];
		if (!trace.m_bInUse) continue;
		if (trace.m_nBlipType != BLIP_CONTACTPOINT) continue;

		if (position.Distance2D(trace.m_vecPos) <= TOLERANCE) return true;
	}
	return false;
}

bool Marker::stillDrawn() const
{
	if (blipHandleOffset == NO_HANDLE_GLOBAL) return isDrawnHere();

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

	if (blipHandleOffset != NO_HANDLE_GLOBAL)
	{
		ScriptGlobals::write(ScriptGlobals::slotOf(blipHandleOffset), handle);
	}

	if (blipDisplay != LEAVE_DISPLAY)
	{
		plugin::Command<eScriptCommands::COMMAND_CHANGE_BLIP_DISPLAY>(handle, blipDisplay);
	}
}

void Marker::remove() const
{
	if (blipHandleOffset == NO_HANDLE_GLOBAL) return;

	int handleSlot = ScriptGlobals::slotOf(blipHandleOffset);
	int handle = ScriptGlobals::read(handleSlot);
	if (handle == 0) return;

	int index = CRadar::GetActualBlipArrayIndex(handle);
	if (index >= 0 && CRadar::ms_RadarTrace[index].m_bInUse) CRadar::ClearBlip(handle);
	ScriptGlobals::write(handleSlot, 0);
}

void Marker::removeIfHere() const
{
	if (blipHandleOffset == NO_HANDLE_GLOBAL) return;

	int handle = ScriptGlobals::readAt(blipHandleOffset);
	if (handle == 0) return;

	int index = CRadar::GetActualBlipArrayIndex(handle);
	if (index < 0 || !CRadar::ms_RadarTrace[index].m_bInUse) return;

	const tRadarTrace& trace = CRadar::ms_RadarTrace[index];
	if (position.Distance2D(trace.m_vecPos) > TOLERANCE) return;

	remove();
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

		if (position.Distance2D(trace.m_vecPos) > TOLERANCE) continue;

		int blip = CRadar::GetNewUniqueBlipIndex(static_cast<int>(i));
		if (blipHandleOffset != NO_HANDLE_GLOBAL && blip == ScriptGlobals::read(handleSlot))
		{
			ScriptGlobals::write(handleSlot, 0);
		}
		CRadar::ClearBlip(blip);
	}
}