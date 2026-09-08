#pragma once
#include "SubmissionTracker.h"

#include <CPed.h>

#include "FightingStyleArbiter.h"

// A gym fight school (Los Santos boxing, San Fierro kung fu, Las Venturas knee-head). Each teaches
// one fighting style once its instructor is beaten. The three differ only in the style they grant
// and the name of their mission script, so one class covers all of them.
class GymTracker : public SubmissionTracker
{
public:
	// t_arbiter is shared by every gym - it decides which earned style is currently active.
	GymTracker(int t_submissionID, eFightingStyle t_taughtStyle, const char* t_scriptName,
		FightingStyleArbiter& t_arbiter);

	void enforceSubmissionReward() override;
	bool pollCompletion() override;

private:
	bool isGymScriptActive() const;

	const eFightingStyle m_taughtStyle;
	const char* m_scriptName;
	FightingStyleArbiter& m_arbiter;

	// The order this gym's style was received in (0 = not received). Compared against the arbiter's
	// latest to decide whether this gym currently owns the style slot.
	int m_receiptOrder = 0;

	// True once the player has been observed inside this gym with our granted style stripped away -
	// from that point until they leave, the taught style appearing on the player can only have been
	// written by the game's own challenge-victory logic.
	bool m_detectionArmed = false;
};
