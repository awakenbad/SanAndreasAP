#include "EdgeCases.h"

#include <CRadar.h>

#include "ScriptGlobals.h"

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

void CatController::update()
{
	writeSkippedMissionState();

	if (counter() == 0)
	{
		Marker date = firstDate();
		date.clearForeign();
		date.raise();
		return;
	}

	firstDate().clearAll();

	if (finished()) return;

	Marker cabin{ positionAt(CABIN_OFFSET, SPOT_STRIDE), readGlobal(CABIN_SPRITE_OFFSET),
		BLIP_HANDLE_OFFSET, Marker::LEAVE_DISPLAY, true };

	cabin.clearForeign();
	cabin.raise();
}

// Everything required to skip King in Exile completely for Catalina
void CatController::writeSkippedMissionState() const
{
	constexpr int TRAILER_STAGE_OFFSET = 2876;
	constexpr int LATER_DATES_OFFSET = 8652;
	constexpr int SAVE_POINT_PROGRESS_OFFSET = 3540;
	constexpr int SAVE_PICKUPS_CREATED_OFFSET = 3536;
	constexpr int STAGE_PENDING = 1;
	constexpr int STAGE_DISPATCHED = 2;
	constexpr int CABIN_SAVE_POINT = 15;

	int stageSlot = ScriptGlobals::slotOf(TRAILER_STAGE_OFFSET);
	if (ScriptGlobals::read(stageSlot) != STAGE_PENDING) return;

	ScriptGlobals::write(stageSlot, STAGE_DISPATCHED);
	ScriptGlobals::write(ScriptGlobals::slotOf(LATER_DATES_OFFSET), 1);

	int saveSlot = ScriptGlobals::slotOf(SAVE_POINT_PROGRESS_OFFSET);
	if (ScriptGlobals::read(saveSlot) < CABIN_SAVE_POINT)
	{
		ScriptGlobals::write(saveSlot, CABIN_SAVE_POINT);
		ScriptGlobals::write(ScriptGlobals::slotOf(SAVE_PICKUPS_CREATED_OFFSET), 0);
	}
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

	if (finished()) return;
	if (readGlobal(BCESAR_COUNTER_OFFSET) < BCESAR_FINISHED) return;

	Marker sanFierro = defaultMarker();
	sanFierro.position = positionAt(SAN_FIERRO_OFFSET);
	sanFierro.handleMayBeStale = true;
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
	if (readGlobal(CATALINA_COUNTER_OFFSET) < CATALINA_ROBBERIES_DONE) return;

	Marker farewell = defaultMarker();
	farewell.position = CVector(-513.9356f, -188.314f, 77.4599f);
	farewell.blipDisplay = Marker::LEAVE_DISPLAY;
	farewell.handleMayBeStale = true;
	farewell.raise();
}
