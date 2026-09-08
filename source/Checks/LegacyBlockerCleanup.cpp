#include "LegacyBlockerCleanup.h"

#include <CObject.h>
#include <CPools.h>
#include <CWorld.h>
#include <common.h>
#include <eModelID.h>

#include "EntityIDs.h"

namespace
{
	constexpr int BLOCKER_MODEL_ID = 2973;
	constexpr int BARRICADE_MODEL_ID = MODEL_CJ_ROADBARRIER;
	constexpr float BARRICADE_Z_OFFSET = 0.6f;
	constexpr float BLOCKER_POSITION_TOLERANCE_SQ = 0.0625f;

	bool sitsOnMarker(const CVector& t_position, int t_modelId)
	{
		for (const Position& spawn : missionStartPos)
		{
			float expectedZ = t_modelId == BARRICADE_MODEL_ID ? spawn.z + BARRICADE_Z_OFFSET : spawn.z;
			float dx = t_position.x - spawn.x;
			float dy = t_position.y - spawn.y;
			float dz = t_position.z - expectedZ;
			if (dx * dx + dy * dy + dz * dz < BLOCKER_POSITION_TOLERANCE_SQ) return true;
		}
		return false;
	}
}

void LegacyBlockerCleanup::removeStaleBlockers()
{
	auto* pool = CPools::ms_pObjectPool;
	if (!pool) return;

	for (int i = 0; i < pool->m_nSize; ++i)
	{
		CObject* object = pool->GetAt(i);
		if (!object) continue;

		int modelId = object->m_nModelIndex;
		if (modelId != BLOCKER_MODEL_ID && modelId != BARRICADE_MODEL_ID) continue;
		if (!sitsOnMarker(object->GetPosition(), modelId)) continue;

		CWorld::Remove(object);
		delete object;
	}
}
