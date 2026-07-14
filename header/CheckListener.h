#pragma once
#include "plugin.h"
#include "CPickups.h"
#include "CMessages.h"
#include <CStats.h>
#include <CWorld.h>

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
	void healthCheckWasReceived();
	void armourCheckWasReceived();
	void fireCheckWasReceived();
private:
	const int PARAMEDIC_ID = 122;
	const int FIREFIGHTER_ID = 123;
	const int VIGILANTE_ID = 124;

	int* m_pickUpCounter;
	int m_lastValuePickUpCounter;
	std::vector<std::string> missions;
	std::string currentMission;
	std::string lastMission;
	int const NO_MISSION = -1;
	bool healthCheckReceived = false;
	bool paramedicCompleted = false;
	bool armourCheckReceived = false;
	bool vigilanteCompleted = false;
	bool fireCheckReceived = false;
	bool firefighterCompleted = false;

	bool tagChecker();
	bool pickUpChecker();
	bool missionChecker();
	void initializeMissionList();
	void enforceSubmissionRewards();
};

