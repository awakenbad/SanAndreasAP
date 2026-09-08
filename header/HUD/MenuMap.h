#pragma once
#include <CMenuManager.h>
#include <CVector.h>
#include <CVector2D.h>
#include <algorithm>

namespace MenuMap
{
	constexpr float MAP_RANGE = 2990.0f;

	inline bool isOpen()
	{
		return FrontEndMenuManager.m_bMenuActive && FrontEndMenuManager.m_nCurrentMenuPage == MENUPAGE_MAP;
	}

	inline CVector2D project(const CVector& t_worldPos)
	{
		float virtualX = FrontEndMenuManager.m_fMapBaseX + FrontEndMenuManager.m_fMapZoom * (t_worldPos.x / MAP_RANGE);
		float virtualY = FrontEndMenuManager.m_fMapBaseY - FrontEndMenuManager.m_fMapZoom * (t_worldPos.y / MAP_RANGE);

		return CVector2D(virtualX * static_cast<float>(RsGlobal.maximumWidth) / 640.0f,
			virtualY * static_cast<float>(RsGlobal.maximumHeight) / 448.0f);
	}

	inline bool worldToScreen(const CVector& t_worldPos, CVector2D& t_screenPos)
	{
		t_screenPos = project(t_worldPos);

		if (t_screenPos.x < 0.0f || t_screenPos.x > static_cast<float>(RsGlobal.maximumWidth)) return false;
		if (t_screenPos.y < 0.0f || t_screenPos.y > static_cast<float>(RsGlobal.maximumHeight)) return false;
		return true;
	}

	inline CVector2D clampToMap(const CVector2D& t_screenPos, float t_inset)
	{
		CVector2D topLeft = project(CVector(-MAP_RANGE, MAP_RANGE, 0.0f));
		CVector2D bottomRight = project(CVector(MAP_RANGE, -MAP_RANGE, 0.0f));

		return CVector2D(std::clamp(t_screenPos.x, topLeft.x + t_inset, bottomRight.x - t_inset),
			std::clamp(t_screenPos.y, topLeft.y + t_inset, bottomRight.y - t_inset));
	}
}
