#include "TagBlipManager.h"
#include <CRadar.h>
#include <CFont.h>
#include <CRGBA.h>
#include <string>

TagBlipManager::TagBlipManager()
{
	m_blipHandles.fill(-1);
}

bool TagBlipManager::update(const std::array<bool, 100>& t_claimed)
{
	bool worldWiped = false;
	if (m_sentinelHandle != -1 && CRadar::GetActualBlipArrayIndex(m_sentinelHandle) < 0)
	{
		worldWiped = true;
		m_sentinelHandle = -1;
	}
	if (m_sentinelHandle == -1)
	{
		m_sentinelHandle = CRadar::SetCoordBlip(BLIP_COORD, CVector(0.0f, 0.0f, 0.0f), 0, BLIP_DISPLAY_NEITHER, nullptr);
	}

	if (m_locatedTagIndex >= 0 && t_claimed[m_locatedTagIndex])
	{
		m_locatedTagIndex = -1;
	}

	if (!m_blipsEnabled)
	{
		for (int& handle : m_blipHandles)
		{
			if (handle != -1 && CRadar::GetActualBlipArrayIndex(handle) >= 0)
			{
				CRadar::ClearBlip(handle);
			}
			handle = -1;
		}
		return worldWiped;
	}

	for (int i = 0; i < static_cast<int>(tagPositions.size()); ++i)
	{
		int& handle = m_blipHandles[i];

		int arrayIndex = handle != -1 ? CRadar::GetActualBlipArrayIndex(handle) : -1;
		if (handle != -1 && arrayIndex < 0)
		{
			handle = -1;
		}

		if (t_claimed[i])
		{
			if (handle != -1)
			{
				CRadar::ClearBlip(handle);
				handle = -1;
			}
			continue;
		}

		if (handle == -1)
		{
			handle = CRadar::SetCoordBlip(BLIP_COORD, tagPositions[i], 0, BLIP_DISPLAY_BOTH, nullptr);
			arrayIndex = handle != -1 ? CRadar::GetActualBlipArrayIndex(handle) : -1;
			if (arrayIndex >= 0)
			{
				CRadar::SetBlipSprite(handle, RADAR_SPRITE_SPRAY);
			}
			else
			{
				handle = -1;
			}
		}

		if (arrayIndex >= 0)
		{
			// Short-range blips (the mechanism shop icons use) still show on the pause map but
			// only appear on the radar when the player is close - without this, all 100 tags
			// clamp to the radar's edge and bury it in icons. The one located tag stays
			// full-range on purpose, making it the only edge-clamped spray icon to follow.
			CRadar::ms_RadarTrace[arrayIndex].m_bShortRange = (i != m_locatedTagIndex);
		}
	}

	return worldWiped;
}

void TagBlipManager::setLocatedTag(int t_tagIndex)
{
	m_locatedTagIndex = (t_tagIndex >= 0 && t_tagIndex < static_cast<int>(tagPositions.size())) ? t_tagIndex : -1;
}

void TagBlipManager::setBlipsEnabled(bool t_enabled)
{
	m_blipsEnabled = t_enabled;
}

void TagBlipManager::drawTagNumbers(const std::array<bool, 100>& t_claimed)
{
	if (!m_blipsEnabled) return;

	CFont::SetFontStyle(FONT_SUBTITLES);
	CFont::SetScale(0.3f, 0.6f);
	CFont::SetColor(CRGBA(255, 255, 255, 255));
	CFont::SetProportional(true);
	CFont::SetOrientation(ALIGN_CENTER);
	CFont::SetDropShadowPosition(1);
	CFont::SetBackground(false, false);

	for (int i = 0; i < static_cast<int>(tagPositions.size()); ++i)
	{
		if (t_claimed[i]) continue;

		CVector2D radarSpace;
		CVector2D worldPos(tagPositions[i].x, tagPositions[i].y);
		CRadar::TransformRealWorldPointToRadarSpace(radarSpace, worldPos);

		// Skip tags outside the visible radar area instead of clamping them to the edge the
		// way native blips do. The radar is a circle of radius 1 in radar space, so clip
		// against that (slightly inside it, since the text has its own extent) rather than
		// the enclosing square - corner positions would render outside the visible disc.
		if (radarSpace.x * radarSpace.x + radarSpace.y * radarSpace.y > 0.85f * 0.85f) continue;

		CVector2D screenPos;
		CRadar::TransformRadarPointToScreenSpace(screenPos, radarSpace);
		CFont::PrintString(screenPos.x, screenPos.y, std::to_string(i + 1).c_str());
	}
}
