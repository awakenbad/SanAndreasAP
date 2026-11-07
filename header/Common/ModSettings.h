#pragma once
#include <CRGBA.h>

namespace ModSettings
{
	enum class ItemColour
	{
		Progression,
		Useful,
		Trap,
		Filler,
		Count,
	};

	enum class CollectibleHeightIndicator
	{
		Off,
		OnBlip,
		Alternating
	};

	constexpr int ITEM_COLOUR_COUNT = static_cast<int>(ItemColour::Count);

	constexpr float NOTIFICATION_MIN = 1.0f;
	constexpr float NOTIFICATION_MAX = 60.0f;
	constexpr float SCALE_MIN = 0.1f;
	constexpr float SCALE_MAX = 2.0f;
	constexpr int DIM_MIN = 10;
	constexpr int DIM_MAX = 100;

	class Values
	{
	public:
		float notificationSeconds;
		float collectibleNumberScale;
		float missionCounterScale;
		CRGBA itemColours[ITEM_COLOUR_COUNT];
		int unselectedDimPercent;
		bool fastTravelEnabled;
		int collectibleHeightIndicator;
	};

	void load();
	Values& values();
	float notificationSeconds();
	float collectibleNumberScale();
	float missionCounterScale();
	const CRGBA& itemColour(ItemColour t_which);
	int unselectedDimPercent();
	bool fastTravelEnabled();
	ModSettings::CollectibleHeightIndicator collectibleHeightIndicatorMode();
}
