#pragma once

// Whether the player is actually driving CJ right now, as opposed to a mission script driving him
// through a cutscene or a scripted walk.
//
// Anything that mutates CJ or his vehicle has to wait for this. A scripted sequence ("walk to the
// car, get in") waits for CJ to arrive; kill him, burst his tires or torch his car halfway there
// and he never arrives, so the script waits forever and the mission softlocks. Observed live: a
// DeathLink landing mid-cutscene left CJ standing still and the cutscene running with no end,
// escapable only because that one happened to be skippable.
namespace PlayerControl
{
	// False during streamed cutscenes, scripted sequences, phone calls, garage waits and any
	// other "player safe" state the scripts set - and also before a player ped exists at all.
	bool isInControl();
}
