#pragma once
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

enum class CheckEvent
{
	None,
	Mission,
	PickUp
};

class CheckListener
{
public:
	CheckListener();
	
	void spawnPickup();
	CheckEvent update();
	std::string getMissionID();
	void submissionCheckWasReceived(int t_submissionID);
private:
	const int PARAMEDIC_ID = 122;
	const int FIREFIGHTER_ID = 123;
	const int VIGILANTE_ID = 124;
	const int BURGLARY_ID = 125;
	const int TAXI_ID = 121;

	int* m_pickUpCounter;
	int m_lastValuePickUpCounter;
	std::vector<std::string> missions;
	std::string currentMission;
	std::string lastMission;
	int const NO_MISSION = -1;
	std::vector<SubmissionTracker*> submissionTrackers;

	bool tagChecker();
	bool pickUpChecker();
	bool missionChecker();
	void initializeMissionList();
	void enforceSubmissionRewards();
};

