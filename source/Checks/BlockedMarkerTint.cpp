#include "BlockedMarkerTint.h"

#include <C3dMarkers.h>
#include <CRGBA.h>
#include <CVector.h>
#include <Patch.h>

#include "BranchProgress.h"
#include "MissionBranches.h"

namespace
{
	constexpr uintptr_t MARKER_CALL_SITES[] = { 0x585CE4, 0x70CD1F, 0x70CD6C, 0x70CDBC };
	constexpr unsigned char BLOCKED_GREY = 0x8C;
	constexpr unsigned char BLOCKED_ALPHA = 130;
	constexpr unsigned char PLACE_MARKER_SET_ALPHA_DIVISOR = 3;
	const BranchProgress* g_progress = nullptr;

	void setMarkerColour(unsigned int t_id, unsigned short t_type, const CRGBA& t_colour)
	{
		for (unsigned int i = 0; i < MAX_NUM_3DMARKERS; ++i)
		{
			C3dMarker& marker = C3dMarkers::m_aMarkerArray[i];
			if (!marker.m_bIsUsed || marker.m_nType != t_type) continue;
			if (marker.m_nIdentifier != static_cast<int>(t_id)) continue;

			marker.m_colour = t_colour;
		}
	}

	void __cdecl onPlaceMarkerSet(unsigned int t_id, unsigned short t_type, CVector& t_posn, float t_size,
		unsigned char t_red, unsigned char t_green, unsigned char t_blue, unsigned char t_alpha,
		unsigned short t_pulsePeriod, float t_pulseFraction, short t_rotateRate)
	{
		C3dMarkers::PlaceMarkerSet(t_id, t_type, t_posn, t_size, t_red, t_green, t_blue, t_alpha,
			t_pulsePeriod, t_pulseFraction, t_rotateRate);

		if (!g_progress) return;

		int marker = missionMarkerIndexAt(t_posn.x, t_posn.y);
		if (marker < 0) return;

		CRGBA colour;
		if (markerIsBlocked(marker, *g_progress))
		{
			colour = CRGBA(BLOCKED_GREY, BLOCKED_GREY, BLOCKED_GREY, BLOCKED_ALPHA);
		}
		else
		{
			colour = CRGBA(t_red, t_green, t_blue, t_alpha / PLACE_MARKER_SET_ALPHA_DIVISOR);
		}

		setMarkerColour(t_id, t_type, colour);
	}
}

void BlockedMarkerTint::install(const BranchProgress& t_progress)
{
	if (g_progress) return;

	g_progress = &t_progress;

	for (uintptr_t site : MARKER_CALL_SITES)
	{
		plugin::patch::RedirectCall(site, &onPlaceMarkerSet);
	}
}
