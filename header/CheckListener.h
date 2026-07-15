#pragma once
#include <array>
#include <queue>
#include "plugin.h"
#include "CPickups.h"
#include "CMessages.h"
#include <CStats.h>
#include <CWorld.h>

#include "SubmissionTracker.h"
#include "ParamedicTracker.h"
#include "FirefighterTracker.h"
#include "VigilanteTracker.h"
#include "BurglaryTracker.h"
#include "TaxiTracker.h"
#include "LosSantosGymTracker.h"

enum class CheckEvent
{
	None,
	Mission,
	PickUp,
	Tag
};

class CheckListener
{
public:
	CheckListener();

	void spawnPickup();
	CheckEvent update();
	std::string getMissionID();
	void submissionCheckWasReceived(int t_submissionID);

	void confirmPickUpSent();
	void confirmMissionSent();

	static bool isStoryMission(int missionId);

	int getPendingTagIndex();
	void confirmTagSent();
private:
	const int PARAMEDIC_ID = 122;
	const int FIREFIGHTER_ID = 123;
	const int VIGILANTE_ID = 124;
	const int BURGLARY_ID = 125;
	const int TAXI_ID = 121;
	const int LOS_SANTOS_GYM_ID = 114;

	int* m_pickUpCounter;
	int m_lastValuePickUpCounter;
	bool m_pickUpEventPending = false;

	std::vector<std::string> missions;
	std::string currentMission;
	std::string lastMission;
	std::string m_pendingMissionName;
	bool m_missionEventPending = false;
	int const NO_MISSION = -1;
	std::vector<SubmissionTracker*> submissionTrackers;

	float m_lastTagCount = 0.0f;
	bool m_tagCountInitialized = false;
	std::queue<int> m_pendingTagIndices;
	std::array<bool, 100> m_tagClaimed{};

	bool tagChecker();
	bool pickUpChecker();
	bool missionChecker();
	void initializeMissionList();
	void enforceSubmissionRewards();
	void findClosestTag(CPlayerPed* player, int delta);
};

