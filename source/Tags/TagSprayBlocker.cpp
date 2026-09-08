#include "TagSprayBlocker.h"

#include <CPlayerPed.h>
#include <CVector.h>
#include <Patch.h>
#include <common.h>

#include "RunningScripts.h"
#include "TagPositions.h"

namespace
{
	constexpr uintptr_t SPRAY_PAINT_WORLD_CALL = 0x73A0FF;
	constexpr uintptr_t SPRAY_PAINT_WORLD_FUNCTION = 0x565B70;

	bool g_locked = false;

	bool isNextToTaggingUpTurfTags()
	{
		CPlayerPed* player = FindPlayerPed();
		if (!player) return false;

		CVector playerPosition = player->GetPosition();

		// Tagging Up Turf Spray Tag IDs
		int tags[] = { 0, 1, 2, 25, 26, 27 };
		for (int tagId : tags)
		{
			if (playerPosition.Distance(tagPositions[tagId]) < 5.0f) return true;
		}

		return false;
	}

	int sprayPaintWorld(CVector* point, CVector* outDir, float radius, char processTagAlphaState)
	{
		if (g_locked)
		{
			if (!RunningScripts::isActive("SWEET1")) return 0;
			if (!isNextToTaggingUpTurfTags()) return 0;
		}

		return reinterpret_cast<int(*)(CVector*, CVector*, float, char)>(SPRAY_PAINT_WORLD_FUNCTION)(point, outDir, radius, processTagAlphaState);
	}
}

void TagSprayBlocker::install()
{
	plugin::patch::RedirectCall(SPRAY_PAINT_WORLD_CALL, &sprayPaintWorld);
}

void TagSprayBlocker::setLocked(bool t_locked)
{
	g_locked = t_locked;
}
