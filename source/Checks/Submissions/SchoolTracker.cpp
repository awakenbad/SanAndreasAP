#include "SchoolTracker.h"

#include "ScriptGlobals.h"

SchoolTracker::SchoolTracker(int t_submissionID, const SubmissionTierSpec& t_spec,
	std::span<const int> t_scoreGlobals)
	: TieredSubmissionTracker(t_submissionID, t_spec), m_scoreGlobals(t_scoreGlobals)
{
}

void SchoolTracker::enforceSubmissionReward()
{
}

int SchoolTracker::testCount() const
{
	return static_cast<int>(m_scoreGlobals.size());
}

TestMedal SchoolTracker::medalForTest(int t_testIndex) const
{
	int score = ScriptGlobals::read(m_scoreGlobals[t_testIndex]);

	if (score >= GOLD_SCORE) return TestMedal::Gold;
	if (score >= SILVER_SCORE) return TestMedal::Silver;
	if (score >= BRONZE_SCORE) return TestMedal::Bronze;
	return TestMedal::None;
}

void SchoolTracker::pollNewTierSlots(std::vector<int>& t_outSlots)
{
	int completed = 0;

	for (int test = 0; test < testCount(); ++test)
	{
		TestMedal earned = medalForTest(test);
		TestMedal sent = m_sent[test];

		for (int level = static_cast<int>(sent) + 1; level <= static_cast<int>(earned); ++level)
		{
			t_outSlots.push_back(SPEC.baseSlot + test * MEDALS_PER_TEST + (level - 1));
		}
		if (earned > sent) m_sent[test] = earned;

		if (earned == TestMedal::Gold) completed++;
	}

	if (completed == testCount() && !submissionCompleted)
	{
		submissionWasCompleted();
	}
}

std::string SchoolTracker::getSentState() const
{
	std::string state;
	for (int test = 0; test < testCount(); ++test)
	{
		state += static_cast<char>('0' + static_cast<int>(m_sent[test]));
	}
	return state;
}

void SchoolTracker::restoreSentState(const std::string& t_state)
{
	for (int test = 0; test < testCount(); ++test)
	{
		char digit = test < static_cast<int>(t_state.size()) ? t_state[test] : '0';
		m_sent[test] = digit >= '0' && digit <= '3'
			? static_cast<TestMedal>(digit - '0')
			: TestMedal::None;
	}
}

float SchoolTracker::getProgress() const
{
	int medals = 0;
	for (int test = 0; test < testCount(); ++test)
	{
		medals += static_cast<int>(medalForTest(test));
	}
	return static_cast<float>(medals);
}
