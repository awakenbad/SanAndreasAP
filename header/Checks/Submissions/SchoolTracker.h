#pragma once
#include "TieredSubmissionTracker.h"

#include <array>
#include <span>

enum class TestMedal
{
	None = 0,
	Bronze = 1,
	Silver = 2,
	Gold = 3,
};

class SchoolTracker : public TieredSubmissionTracker
{
public:
	SchoolTracker(int t_submissionID, const SubmissionTierSpec& t_spec,
		std::span<const int> t_scoreGlobals);
	void enforceSubmissionReward() override;

	void pollNewTierSlots(std::vector<int>& t_outSlots) override;

	std::string getSentState() const override;
	void restoreSentState(const std::string& t_state) override;

protected:
	float getProgress() const override;

	virtual TestMedal medalForTest(int t_testIndex) const;

	int testCount() const;

private:
	static constexpr int MEDALS_PER_TEST = 3;

	static constexpr int BRONZE_SCORE = 70;
	static constexpr int SILVER_SCORE = 85;
	static constexpr int GOLD_SCORE = 100;

	static constexpr int MAX_TESTS = 12;

	std::array<TestMedal, MAX_TESTS> m_sent{};
	std::span<const int> m_scoreGlobals;
};
