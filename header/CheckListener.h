#pragma once
#include <array>
#include <string>
#include <memory>
#include "plugin.h"
#include "PendingChecks.h"
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
	Tag,
	Submission
};

class CheckListener
{
public:
	CheckListener();

	CheckEvent update();
	std::string getMissionID();
	void submissionCheckWasReceived(int t_submissionID);

	void confirmPickUpSent();
	void confirmMissionSent();

	static bool isStoryMission(int missionId);

	int getPendingTagIndex();
	void confirmTagSent();

	const std::array<bool, 100>& getClaimedTags() const;

	// TEMPORARY: live tag-detection internals for the roof-spraycan investigation.
	std::string tagDebugLine() const;

	// TEMPORARY: puts local state where a finished Los Santos would have it - all tags claimed
	// (clears their blips, persists) and all submissions completed (stat latches won't refire,
	// vanilla rewards suppressed until items arrive). Part of the send-all-LS-checks dev tool.
	void debugCompleteLosSantos();

	// TEMPORARY: raw LastMissionPassedName plus the mission ID our table resolves it to
	// (-1 = not in the table), for verifying the Badlands mission IDs in-game.
	std::string missionDebugLine() const;
	void restoreClaimedTags(const std::array<bool, 100>& t_claimed);

	const std::vector<std::unique_ptr<SubmissionTracker>>& getSubmissionTrackers() const;

	void resyncBaselines();

	int getPendingSubmissionId();
	void confirmSubmissionSent();

	// Levelled submissions (Paramedic/Firefighter/Vigilante) send a check per level reached
	// rather than one on completion - see LevelledSubmissionTracker for the slot numbering.
	bool hasPendingSubmissionLevel() const;
	int getPendingSubmissionLevelSlot() const;
	void confirmSubmissionLevelSent();
private:
	static constexpr uintptr_t TAXI_FARES_ADDR = 0xA49C30;
	static constexpr int32_t TAXI_FARES_FOR_COMPLETION = 50;

	// Vanilla grants the infinite sprint reward once $10000 worth of goods has been stolen
	// across burglary runs (STAT_MONEY_MADE_FROM_BURGLARY tracks it).
	static constexpr float BURGLARY_LOOT_FOR_COMPLETION = 10000.0f;

	int* m_pickUpCounter;
	int m_lastValuePickUpCounter;

	std::vector<std::string> missions;
	std::string currentMission;
	std::string lastMission;
	int const NO_MISSION = -1;
	std::vector<std::unique_ptr<SubmissionTracker>> submissionTrackers;

	float m_lastTagCount = 0.0f;
	bool m_tagCountInitialized = false;
	bool m_baselinesInitialized = false;
	std::array<bool, 100> m_tagClaimed{};

	PendingChecks<int> m_pendingPickUps;
	PendingChecks<std::string> m_pendingMissions;
	PendingChecks<int> m_pendingTags;
	PendingChecks<int> m_pendingSubmissions;
	PendingChecks<int> m_pendingSubmissionLevels;

	SubmissionTracker* findTracker(int t_submissionID);
	bool tagChecker();
	bool pickUpChecker();
	bool missionChecker();
	bool submissionLevelChecker();
	void initializeMissionList();
	void enforceSubmissionRewards();
	void findClosestTag(CPlayerPed* player, int delta);
};

