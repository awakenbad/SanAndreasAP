#include "PlayerControl.h"
#include "common.h"
#include <CPad.h>
#include <CCutsceneMgr.h>

bool PlayerControl::isInControl()
{
	// No ped means no one to act on - during a load, or between worlds.
	if (!FindPlayerPed()) return false;

	// Covers the streamed .cuts cutscenes, which are only some of them.
	if (CCutsceneMgr::ms_running) return false;

	// Covers the rest: in-game scripted sequences take control away by setting one of the bits in
	// this union (bPlayerSafeForCutscene, bPlayerSafe, bPlayerSkipsToDestination,
	// bPlayerTalksOnPhone, ...). Testing the whole word for zero catches every one of them without
	// having to know which flag a given mission happens to use.
	CPad* pad = CPad::GetPad(0);
	if (pad && pad->DisablePlayerControls != 0) return false;

	return true;
}
