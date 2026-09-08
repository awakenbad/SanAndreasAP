#include "CourierTracker.h"

#include "RunningScripts.h"
#include "ScriptGlobals.h"

namespace
{
	constexpr char COURIER_SCRIPT[] = "BCOUR";

	constexpr int CITY_LOCAL = 872;
	constexpr int LEVEL_LOCAL = 746;

	constexpr int FIRST_LEVEL_STATE = 1;
	constexpr int LAST_LEVEL_STATE = 4;
	constexpr int LEVEL_COUNT = 4;
}

CourierTracker::CourierTracker(int t_submissionID, const SubmissionTierSpec& t_spec, int t_cityId,
	int t_passedGlobal)
	: TieredSubmissionTracker(t_submissionID, t_spec), m_cityId(t_cityId), m_passedGlobal(t_passedGlobal)
{
}

void CourierTracker::enforceSubmissionReward()
{
}

bool CourierTracker::isCourierScriptActive() const
{
	return RunningScripts::isActive(COURIER_SCRIPT);
}

float CourierTracker::getProgress() const
{
	if (ScriptGlobals::read(m_passedGlobal) == 1) return static_cast<float>(LEVEL_COUNT);

	if (!isCourierScriptActive()) return 0.0f;
	if (CTheScripts::LocalVariablesForCurrentMission[CITY_LOCAL].iParam != m_cityId) return 0.0f;

	int state = CTheScripts::LocalVariablesForCurrentMission[LEVEL_LOCAL].iParam;
	if (state < FIRST_LEVEL_STATE || state > LAST_LEVEL_STATE) return 0.0f;

	return static_cast<float>(state - FIRST_LEVEL_STATE);
}
