#pragma once
#include "SubmissionTracker.h"
#include <CPed.h>

class LosSantosGymTracker : public SubmissionTracker
{
public:
	LosSantosGymTracker(int t_submissionID);
	void enforceSubmissionReward() override;
	bool pollCompletion();
private:
	bool isGymScriptActive() const;

	// True once the player has been observed inside the LS gym with our granted boxing style
	// stripped away - from that point until they leave, any STYLE_BOXING appearing on the player
	// can only have been written by the game's own challenge-victory logic.
	bool m_detectionArmed = false;
};
