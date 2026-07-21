#include "TieredSubmissionTracker.h"

TieredSubmissionTracker::TieredSubmissionTracker(int t_submissionID, const SubmissionTierSpec& t_spec)
	: SubmissionTracker(t_submissionID), SPEC(t_spec)
{
}

void TieredSubmissionTracker::pollNewTierSlots(std::vector<int>& t_outSlots)
{
	// Progress only ever climbs, so emit every tier crossed since the last poll - normally one,
	// but loading a save (or a single lucrative burglary run) can cross several at once.
	int tier = static_cast<int>(getProgress() / SPEC.progressPerTier);
	if (tier > SPEC.tierCount) tier = SPEC.tierCount;

	for (int reached = m_sentTier + 1; reached <= tier; ++reached)
	{
		t_outSlots.push_back(SPEC.baseSlot + (reached - 1));
	}
	if (tier > m_sentTier)
	{
		m_sentTier = tier;
	}

	// Completion latches on the final tier - enforceSubmissionReward() uses it to decide whether
	// vanilla's reward should stay suppressed until the AP item arrives.
	if (tier >= SPEC.tierCount && !submissionCompleted)
	{
		submissionWasCompleted();
	}
}

int TieredSubmissionTracker::getSentTier() const
{
	return m_sentTier;
}

void TieredSubmissionTracker::restoreSentTier(int t_tier)
{
	m_sentTier = t_tier;
}
