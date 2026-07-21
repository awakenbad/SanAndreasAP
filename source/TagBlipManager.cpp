#include "TagBlipManager.h"
#include "ScreenScale.h"
#include "common.h"
#include <algorithm>
#include <climits>
#include <vector>
#include <CRadar.h>
#include <CFont.h>
#include <CRGBA.h>
#include <string>

TagBlipManager::TagBlipManager()
{
	m_blipHandles.fill(-1);
}

int TagBlipManager::tagIndexAt(const CVector& t_pos) const
{
	for (int i = 0; i < static_cast<int>(tagPositions.size()); ++i)
	{
		float dx = t_pos.x - tagPositions[i].x;
		float dy = t_pos.y - tagPositions[i].y;
		if (dx * dx + dy * dy < 4.0f) return i;
	}
	return -1;
}

int TagBlipManager::findExistingBlipForTag(int t_tagIndex) const
{
	for (unsigned int i = 0; i < MAX_RADAR_TRACES; ++i)
	{
		const tRadarTrace& trace = CRadar::ms_RadarTrace[i];
		if (!trace.m_bInUse) continue;
		if (tagIndexAt(trace.m_vecPos) != t_tagIndex) continue;

		return CRadar::GetNewUniqueBlipIndex(static_cast<int>(i));
	}
	return -1;
}

void TagBlipManager::reconcileWithPool()
{
	m_blipHandles.fill(-1);

	for (unsigned int i = 0; i < MAX_RADAR_TRACES; ++i)
	{
		const tRadarTrace& trace = CRadar::ms_RadarTrace[i];
		if (!trace.m_bInUse) continue;

		int tagIndex = tagIndexAt(trace.m_vecPos);
		if (tagIndex < 0) continue;

		int handle = CRadar::GetNewUniqueBlipIndex(static_cast<int>(i));
		if (m_blipHandles[tagIndex] == -1)
		{
			m_blipHandles[tagIndex] = handle; // adopt the one the save restored
		}
		else
		{
			CRadar::ClearBlip(handle); // duplicate from an earlier load - drop it
		}
	}
}

// A blip handle only means anything inside the session that created it. A load restores the
// radar pool from the save, so a stale handle can still resolve - but to whatever blip the save
// put in that slot. Clearing that destroys someone else's blip, and for a mission marker that
// takes the red world marker with it, permanently.
bool TagBlipManager::ownsBlip(int t_handle) const
{
	if (t_handle == -1) return false;

	int arrayIndex = CRadar::GetActualBlipArrayIndex(t_handle);
	if (arrayIndex < 0) return false;

	return isOurTagPosition(CRadar::ms_RadarTrace[arrayIndex].m_vecPos);
}

void TagBlipManager::onWorldWiped()
{
	m_sentinelHandle = -1;
	m_reconcileTicks = RECONCILE_TICKS;
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

	// Our blips are written into the save file, so any load - including one straight after
	// launching the game, where there is no previous world to detect a wipe against - brings a
	// full set back. Creating another set on top doubles the count each time, and the radar pool
	// is only 175 traces; once it is full the game can no longer create ITS OWN blips, so
	// mission markers silently stop appearing. Reconciling against what is actually in the world
	// makes this idempotent, and repairs saves that already accumulated duplicates.
	if (m_reconcileTicks > 0 && FindPlayerPed())
	{
		reconcileWithPool();
		m_reconcileTicks--;
	}

	if (m_locatedTagIndex >= 0 && t_claimed[m_locatedTagIndex])
	{
		m_locatedTagIndex = -1;
	}

	if (!m_blipsEnabled)
	{
		for (int& handle : m_blipHandles)
		{
			if (ownsBlip(handle))
			{
				CRadar::ClearBlip(handle);
			}
			handle = -1;
		}
		return worldWiped;
	}

	CPlayerPed* player = FindPlayerPed();
	if (!player) return worldWiped;

	// Blipping all 100 tags would eat 100 of the game's 175 radar traces, leaving too little
	// headroom for the game's own blips late in the story - and running out is invisible, the
	// game just silently fails to create a mission marker. Only the nearest few are blipped;
	// the rest cost nothing until the player gets near them.
	std::array<int, 100> rank;
	rank.fill(INT_MAX);
	{
		CVector playerPos = player->GetPosition();
		std::vector<std::pair<float, int>> byDistance;
		byDistance.reserve(tagPositions.size());

		for (int i = 0; i < static_cast<int>(tagPositions.size()); ++i)
		{
			if (t_claimed[i]) continue;
			float dx = playerPos.x - tagPositions[i].x;
			float dy = playerPos.y - tagPositions[i].y;
			byDistance.push_back({ dx * dx + dy * dy, i });
		}
		std::sort(byDistance.begin(), byDistance.end());

		for (int r = 0; r < static_cast<int>(byDistance.size()); ++r)
		{
			rank[byDistance[r].second] = r;
		}
	}

	for (int i = 0; i < static_cast<int>(tagPositions.size()); ++i)
	{
		int& handle = m_blipHandles[i];

		// Drop any handle we can't prove still refers to one of our own blips.
		if (handle != -1 && !ownsBlip(handle))
		{
			handle = -1;
		}
		int arrayIndex = handle != -1 ? CRadar::GetActualBlipArrayIndex(handle) : -1;

		// The located tag (from /tag) is always blipped so it stays followable from anywhere.
		// Tags that already have a blip keep it a little past the cut-off, so one sitting right
		// on the boundary doesn't flicker on and off as the player moves.
		bool wantBlip = false;
		if (i == m_locatedTagIndex) wantBlip = true;
		else if (rank[i] < MAX_TAG_BLIPS) wantBlip = true;
		else if (handle != -1 && rank[i] < MAX_TAG_BLIPS + BLIP_HYSTERESIS) wantBlip = true;

		if (t_claimed[i] || !wantBlip)
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
			// Blips persist in save files, so this tag's blip may already be in the world. Adopt
			// it instead of adding a second one - duplicates are what exhaust the 175-trace pool
			// and stop the game creating its own mission-marker blips. Checking here rather than
			// only on load makes it immune to when the save's radar data actually lands.
			handle = findExistingBlipForTag(i);
			if (handle == -1)
			{
				handle = CRadar::SetCoordBlip(BLIP_COORD, tagPositions[i], 0, BLIP_DISPLAY_BOTH, nullptr);
				if (handle != -1 && CRadar::GetActualBlipArrayIndex(handle) >= 0)
				{
					CRadar::SetBlipSprite(handle, RADAR_SPRITE_SPRAY);
				}
				else
				{
					handle = -1;
				}
			}
			arrayIndex = handle != -1 ? CRadar::GetActualBlipArrayIndex(handle) : -1;
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

void TagBlipManager::toggleBlips()
{
	m_blipsEnabled = !m_blipsEnabled;
}

bool TagBlipManager::areBlipsEnabled() const
{
	return m_blipsEnabled;
}

void TagBlipManager::drawTagNumbers(const std::array<bool, 100>& t_claimed)
{
	if (!m_blipsEnabled) return;

	CFont::SetFontStyle(FONT_SUBTITLES);
	CFont::SetScale(ScreenScale::of(0.3f), ScreenScale::of(0.6f));
	CFont::SetColor(CRGBA(255, 255, 255, 255));
	CFont::SetProportional(true);
	CFont::SetOrientation(ALIGN_CENTER);
	CFont::SetDropShadowPosition(1);
	CFont::SetBackground(false, false);

	for (int i = 0; i < static_cast<int>(tagPositions.size()); ++i)
	{
		// Only tags that actually have a blip get a number, so a number can never appear with no
		// icon underneath it now that distant tags aren't blipped.
		if (t_claimed[i] || m_blipHandles[i] == -1) continue;

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
