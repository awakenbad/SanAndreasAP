#include "EdgeCases.h"
#include "ScriptGlobals.h"
#include "ScriptSlice.h"
#include "PhoneCall.h"
#include <CRadar.h>

namespace
{
	constexpr int MOB_SF = 183113;
	constexpr int MOB_VEG = 185175;

	constexpr ScriptSlice TORENO_START_CALL = { MOB_SF + 1357, MOB_SF + 1451 };
	constexpr ScriptSlice CASINO_START_CALL = { MOB_SF + 1850, MOB_SF + 2027 };
	constexpr ScriptSlice WUZI_START_CALL = { MOB_SF + 291, MOB_SF + 328 };

	constexpr int TORENO_CALLED_OFFSET = 5616;
	constexpr int CASINO_CALLED_OFFSET = 5612;
	constexpr int WUZI_CALLED_OFFSET = 5604;

	constexpr int DESERT_COUNTER_OFFSET = 2372;

	constexpr PhoneCall TORENO_CALLS[] = {
		{ 5600, DESERT_COUNTER_OFFSET, 1, { MOB_SF + 1565, MOB_SF + 1593 } },
		{ 5608, DESERT_COUNTER_OFFSET, 2, { MOB_SF + 1707, MOB_SF + 1736 } },
		{ 5556, DESERT_COUNTER_OFFSET, 8, { MOB_VEG + 255, MOB_VEG + 284 } },
	};
}

BcrashController::BcrashController()
	: EdgeCase({ "BCRASH", 65160, 1972, 1, 1976, 1776, 1936,
		Marker::LEAVE_DISPLAY, NO_PREREQUISITE })
{
}

void BcrashController::update()
{
	Marker trailer = defaultMarker();
	trailer.handleMayBeStale = true;

	if (finished())
	{
		trailer.clearAll();
		return;
	}

	trailer.clearForeign();
	trailer.raise();
}

CatController::CatController()
	: EdgeCase({ "CAT", 65347, 256, 4, 0, 0, BLIP_HANDLE_OFFSET,
		Marker::LEAVE_DISPLAY, NO_PREREQUISITE })
{
}

// The one blip in the script that uses a literal instead of a global
Marker CatController::firstDate() const
{
	return { positionAt(FIRST_DATE_OFFSET, SPOT_STRIDE), RADAR_SPRITE_QMARK,
		BLIP_HANDLE_OFFSET, Marker::LEAVE_DISPLAY, true };
}

Marker CatController::cabin() const
{
	return { positionAt(CABIN_OFFSET, SPOT_STRIDE), ScriptGlobals::readAt(CABIN_SPRITE_OFFSET),
		BLIP_HANDLE_OFFSET, Marker::LEAVE_DISPLAY, true };
}

void CatController::update()
{
	writeSkippedMissionState();

	if (counter() == 0)
	{
		cabin().clearAll();

		Marker date = firstDate();
		date.clearForeign();
		date.raise();
		return;
	}

	firstDate().clearAll();

	if (finished()) return;

	Marker robbery = cabin();
	robbery.clearForeign();
	robbery.raise();
}

// Everything required to skip King in Exile completely for Catalina
void CatController::writeSkippedMissionState() const
{
	constexpr int TRAILER_STAGE_OFFSET = 2876;
	constexpr int LATER_DATES_OFFSET = 8652;
	constexpr int STAGE_PENDING = 1;
	constexpr int STAGE_DISPATCHED = 2;

	int stageSlot = ScriptGlobals::slotOf(TRAILER_STAGE_OFFSET);
	if (ScriptGlobals::read(stageSlot) != STAGE_PENDING) return;

	ScriptGlobals::write(stageSlot, STAGE_DISPATCHED);
	ScriptGlobals::write(ScriptGlobals::slotOf(LATER_DATES_OFFSET), 1);
}

TruController::TruController()
	: EdgeCase({ "TRU", 66412, 1964, 2, 2000, 1952, 1944,
		Marker::LEAVE_DISPLAY, NO_PREREQUISITE })
{
}

void TruController::update()
{
	if (counter() == 0)
	{
		defaultMarker().raise();
		return;
	}

	Marker sanFierro = defaultMarker();
	sanFierro.position = positionAt(SAN_FIERRO_OFFSET);
	sanFierro.handleMayBeStale = true;

	if (finished())
	{
		sanFierro.clearAll();
		return;
	}

	sanFierro.raise();
}

BcesarController::BcesarController()
	: EdgeCase({ "BCESAR", 66700, 1968, 10, 2012, 1788, 1940,
		Marker::BLIP_ONLY, NO_PREREQUISITE })
{
}

void BcesarController::update()
{
	if (counter() == 0)
	{
		defaultMarker().raise();
		return;
	}

	if (counter() < FAREWELL_FIRST || counter() > FAREWELL_LAST) return;

	Marker farewell = defaultMarker();
	farewell.position = CVector(-513.9356f, -188.314f, 77.4599f);
	farewell.blipDisplay = Marker::LEAVE_DISPLAY;
	farewell.handleMayBeStale = true;
	farewell.raise();
}

GarageController::GarageController()
	: EdgeCase({ "GARAGE", 67587, 2164, 2, 2188, 0, 2132,
		Marker::LEAVE_DISPLAY, NO_PREREQUISITE })
{
}

Marker GarageController::garage() const
{
	return { positionAt(m_row.positionOffset), RADAR_SPRITE_CJ,
		m_row.blipHandleOffset, Marker::LEAVE_DISPLAY };
}

void GarageController::update()
{
	if (!finished())
	{
		garage().raise();
		return;
	}

	if (ScriptGlobals::readAt(SCRASH_COUNTER_OFFSET) < SCRASH_FINISHED) return;
	if (ScriptGlobals::readAt(SYND_COUNTER_OFFSET) < SYND_FINISHED) return;

	garage().clearAll();
}

WuziController::WuziController()
	: EdgeCase({ "WUZI", 68090, 2172, 5, 2212, 2136, 2096,
		Marker::LEAVE_DISPLAY, NO_PREREQUISITE })
{
}

void WuziController::update()
{
	if (!positionsInitialised()) return;

	if (ScriptGlobals::readAt(WUZI_CALLED_OFFSET) == 0) WUZI_START_CALL.run();
	if (!finished()) defaultMarker().raise();
}

TorenoController::TorenoController()
	: EdgeCase({ "DESERT", 70939, 2372, 9, 2404, 2344, 2320,
		Marker::LEAVE_DISPLAY, NO_PREREQUISITE })
{
}

void TorenoController::update()
{
	if (!positionsInitialised()) return;

	if (ScriptGlobals::readAt(TORENO_CALLED_OFFSET) == 0) TORENO_START_CALL.run();

	for (const PhoneCall& call : TORENO_CALLS)
	{
		call.runIfDue();
	}
}

CasinoController::CasinoController()
	: EdgeCase({ "CASINO", 71913, 2388, 9, 2428, 2352, 2328,
		Marker::LEAVE_DISPLAY, NO_PREREQUISITE })
{
}

void CasinoController::update()
{
	if (!positionsInitialised()) return;

	if (ScriptGlobals::readAt(CASINO_CALLED_OFFSET) == 0) CASINO_START_CALL.run();
}

VegasCrashController::VegasCrashController()
	: EdgeCase({ "VCRASH", 72627, 2392, 2, 2452, 1776, 2332,
		Marker::LEAVE_DISPLAY, NO_PREREQUISITE })
{
}

void VegasCrashController::update()
{
	if (counter() == 0)
	{
		defaultMarker().raise();
		return;
	}

	defaultMarker().clearAll();
}

MaddDoggController::MaddDoggController()
	: EdgeCase({ "DOC", 72921, 2396, 1, 2464, 2356, 2336,
		Marker::LEAVE_DISPLAY, NO_PREREQUISITE })
{
}

void MaddDoggController::update()
{
	if (finished())
	{
		defaultMarker().clearAll();
		return;
	}

	defaultMarker().raise();
}
