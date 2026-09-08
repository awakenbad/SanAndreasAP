#pragma once
#include "SubmissionTracker.h"

#include <array>

#include "EntityIDs.h"

class RaceTracker : public SubmissionTracker
{
public:
	RaceTracker(int t_submissionID, const SubmissionTierSpec& t_spec, int t_firstRaceIndex);

	void enforceSubmissionReward() override {}

	void pollNewTierSlots(std::vector<int>& t_outSlots) override;

private:
	static constexpr int MAX_RACES_PER_GROUP = 10;

	const SubmissionTierSpec& SPEC;
	const int FIRST_RACE_INDEX;

	std::array<bool, MAX_RACES_PER_GROUP> m_sent{};
};
