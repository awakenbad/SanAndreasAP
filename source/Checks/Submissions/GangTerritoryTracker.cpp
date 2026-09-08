#include "GangTerritoryTracker.h"

#include <CGangWars.h>

#include "ParseUtils.h"

GangTerritoryTracker::GangTerritoryTracker(int t_submissionID)
	: TieredSubmissionTracker(t_submissionID, GANG_TERRITORY_TIERS)
{
}

void GangTerritoryTracker::enforceSubmissionReward()
{
}

float GangTerritoryTracker::getProgress() const
{
	CGangWars::UpdateTerritoryUnderControlPercentage();
	float pct = CGangWars::TerritoryUnderControlPercentage * 100.0f;

	if (pct < GANG_TERRITORY_TIERS.progressPerTier) m_armed = true;
	return m_armed ? pct : 0.0f;
}

void GangTerritoryTracker::restoreSentState(const std::string& t_state)
{
	TieredSubmissionTracker::restoreSentState(t_state);
	m_armed = parseIntOr(t_state, 0) > 0;
}
