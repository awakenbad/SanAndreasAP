#include "CityUnlock.h"
#include <CStats.h>
#include <CIplStore.h>
#include <IplDef.h>
#include <eScriptCommands.h>
#include <extensions/ScriptCommands.h>
#include "ScriptCommandHook.h"
#include <CRunningScript.h>
#include "RunningScripts.h"
#include "ScriptGlobals.h"

// Taken from ChaosModComplementaries. Thanks to Lordmau5 for letting me use this!

using namespace plugin;

namespace
{
	constexpr int STAT_CITIES_PASSED = 181;
	constexpr int RETURN_CITIES_PASSED_ID = 25;
	constexpr int ALL_CITIES_PASSED = 3;

	class RoadArea
	{
	public:
		float minX, minY, minZ;
		float maxX, maxY, maxZ;
		bool on;
	};

	constexpr RoadArea ROAD_AREAS[] = {
		{ -2696.464f, 1239.866f, 40.7599f, -2665.359f, 2190.96f, 70.8125f, true },
		{ -2740.694f, 2233.618f, 40.8431f, -2720.91f, 2338.224f, 80.4822f, true },
		{ -2695.584f, 1237.981f, 40.7328f, -2664.417f, 1454.767f, 60.8126f, true },
		{ -995.0013f, -416.2032f, 30.4207f, -940.5399f, -251.8564f, 40.6762f, true },
		{ -205.8387f, 250.7443f, 7.2472f, -131.0039f, 481.8496f, 15.9152f, true },
		{ -100.7515f, -927.8298f, 18.0f, -68.3752f, -891.9871f, 14.0f, true },
		{ 609.7595f, 327.3437f, 15.8783f, 429.8884f, 616.0168f, 20.289f, true },
		{ 317.1688f, 707.7672f, 7.0f, 437.5726f, 709.0657f, 20.5578f, true },
		{ 391.1194f, 640.015f, 7.0f, 402.2627f, 664.798f, 18.5098f, true },
		{ 289.2904f, 636.3991f, 7.8675f, 409.4943f, 702.3849f, 20.0345f, true },
		{ 300.3153f, 718.7909f, 7.7846f, 316.7906f, 781.0926f, 14.0795f, true },
		{ 254.9982f, 837.029f, 10.1731f, 270.7453f, 929.2404f, 30.2553f, true },
		{ 210.7811f, 939.2068f, 10.9062f, 249.4799f, 959.1111f, 30.2141f, true },
		{ 230.4545f, 946.0961f, 20.6674f, 255.9772f, 969.2755f, 30.4776f, true },
		{ 249.4279f, 899.7975f, 10.5871f, 268.6826f, 933.5995f, 30.3975f, true },
		{ 312.1081f, 694.1089f, 6.0f, 324.0811f, 733.0005f, 10.0f, true },
		{ 324.4526f, 804.9198f, 9.6186f, 332.8747f, 814.356f, 14.3925f, true },
		{ 1690.819f, 376.5103f, 28.1103f, 1730.223f, 445.2955f, 30.8414f, true },
		{ 1643.536f, 227.3723f, 27.4457f, 1673.062f, 295.5788f, 30.0815f, true },
		{ 1673.765f, 388.1013f, 40.2331f, 1815.862f, 804.9291f, 10.0f, true },
		{ 1705.156f, 308.3448f, 20.0f, 1710.948f, 316.4094f, 23.5612f, true },
		{ -12.7067f, -1522.455f, 1.0f, 80.8463f, -1517.111f, 5.0f, true },
		{ -16.3392f, -1532.882f, 0.0394f, 69.3401f, -1523.771f, 5.922f, true },
		{ 618.7253f, -1189.606f, 18.0f, 623.5441f, -1161.981f, 22.0f, true },
		{ -33.4208f, -1341.84f, 9.0f, 35.3764f, -1303.948f, 13.0f, true },
		{ -41.2393f, -1385.87f, 8.0f, -3.5883f, -1368.856f, 10.5f, true },
		{ -1690.705f, 539.6102f, 30.3278f, -1100.567f, 1140.569f, 50.735f, true },
		{ -1799.541f, 379.7155f, 16.0f, -1780.199f, 392.2779f, 18.0f, true },
		{ -1092.429f, 1286.505f, 30.0f, -1077.038f, 1319.495f, 35.0f, true },
		{ -1860.133f, 314.7891f, 38.0f, -1638.563f, 557.4354f, 40.0f, true },
		{ -1737.333f, 455.9431f, 30.3573f, -1710.363f, 500.6261f, 40.4891f, true },
		{ -1689.229f, 513.0995f, 30.2597f, -1679.124f, 524.8383f, 40.25f, true },
		{ -1742.906f, 500.7302f, 30.4679f, -1650.312f, 551.8201f, 40.7455f, true },
		{ -1761.95f, 507.8931f, 35.0533f, -1751.361f, 531.5917f, 41.3335f, false },
	};

	bool barrierIplLoaded(const char* t_name)
	{
		int slot = CIplStore::FindIplSlot(t_name);
		if (slot < 0) return false;

		IplDef* def = CIplStore::ms_pPool->GetAt(slot);
		if (!def) return false;

		return def->field_2D != 0 && def->m_bDisableDynamicStreaming;
	}

	void restoreRoads()
	{
		for (const RoadArea& area : ROAD_AREAS)
		{
			if (area.on)
			{
				Command<eScriptCommands::COMMAND_SWITCH_ROADS_ON>(
					area.minX, area.minY, area.minZ, area.maxX, area.maxY, area.maxZ);
			}
			else
			{
				Command<eScriptCommands::COMMAND_SWITCH_ROADS_OFF>(
					area.minX, area.minY, area.minZ, area.maxX, area.maxY, area.maxZ);
			}
		}
	}

	void removeBarriers()
	{
		bool firstLoaded = barrierIplLoaded("BARRIERS1");
		bool secondLoaded = barrierIplLoaded("BARRIERS2");
		if (!firstLoaded && !secondLoaded) return;

		if (firstLoaded) Command<eScriptCommands::COMMAND_REMOVE_IPL>("BARRIERS1");
		if (secondLoaded) Command<eScriptCommands::COMMAND_REMOVE_IPL>("BARRIERS2");

		restoreRoads();
	}

	bool blockUnlockedCityGate(CRunningScript* t_script)
	{
		if (_strnicmp(t_script->m_szName, "MOB_LA1", 8) != 0) return false;

		tScriptParam* destination = t_script->GetPointerToScriptVariable(2);
		if (destination == ScriptGlobals::address(RETURN_CITIES_PASSED_ID))
		{
			t_script->UpdateCompareFlag(false);
			return true;
		}

		return false;
	}

	bool blockMissionCallsTermination(CRunningScript* t_script)
	{
		return _strnicmp(t_script->m_szName, "MOB_LA1", 8) == 0;
	}
}

void CityUnlock::update()
{
	ScriptCommandHook::blockCommand(COMMAND_IS_INT_VAR_GREATER_THAN_NUMBER, &blockUnlockedCityGate);
	ScriptCommandHook::blockCommand(COMMAND_TERMINATE_THIS_SCRIPT, &blockMissionCallsTermination);

	if (static_cast<int>(CStats::GetStatValue(STAT_CITIES_PASSED)) < ALL_CITIES_PASSED)
	{
		CStats::SetStatValue(STAT_CITIES_PASSED, static_cast<float>(ALL_CITIES_PASSED));
	}

	if (!RunningScripts::isActive("MOB_LA1"))
	{
		unsigned char* scriptBase = reinterpret_cast<unsigned char*>(CTheScripts::ScriptSpace);
		CTheScripts::StartNewScript(scriptBase + 180158);
	}

	removeBarriers();
}
