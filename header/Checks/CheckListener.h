#pragma once
#include "PersistentState.h"

#include <array>
#include <memory>
#include <string>

#include "ExportListTracker.h"
#include "FightingStyleArbiter.h"
#include "HorseshoeTracker.h"
#include "OysterTracker.h"
#include "PendingChecks.h"
#include "SnapshotTracker.h"
#include "StuntJumpTracker.h"
#include "SubmissionTracker.h"
#include "TagTracker.h"

enum class CheckEvent
{
	None,
	Mission,
	Submission
};

class CheckListener : public PersistentState
{
public:
	CheckListener();

	void save(SaveDataManager& t_saveData) override;
	void load(const SaveDataManager& t_saveData) override;

	CheckEvent update();
	std::string getMissionID();
	void submissionCheckWasReceived(int t_submissionID);
	void submissionUnlockWasReceived(int t_submissionID);
	void collectibleUnlockWasReceived(const std::string& t_checkType);
	void setGatedContent(const std::string& t_effectNames);

	void confirmMissionSent();

	static bool isStoryMission(int missionId);

	const std::array<CollectibleTracker*, 6>& getCollectibles() const { return m_collectibles; }

	void locateCollectible(const std::string& t_type, int t_index);

	void setIncludedCollectibles(const std::string& t_types);

	int getPendingSubmissionId();
	void confirmSubmissionSent();

	bool hasPendingSubmissionLevel() const;
	int getPendingSubmissionLevelSlot() const;
	void confirmSubmissionLevelSent();
private:
	std::vector<std::string> missions;
	std::string currentMission;
	std::string lastMission;
	int const NO_MISSION = -1;

	static constexpr char ENDING_CUTSCENE_NAME[] = "RIOT4E1";
	static constexpr int END_OF_THE_LINE_ID = 112;
	bool m_endingFired = false;
	bool isEndingCutscenePlaying() const;
	FightingStyleArbiter m_styleArbiter;
	std::vector<std::unique_ptr<SubmissionTracker>> submissionTrackers;

	bool m_baselinesInitialized = false;

	TagTracker m_tagTracker;
	StuntJumpTracker m_stuntJumpTracker;
	SnapshotTracker m_snapshotTracker;
	HorseshoeTracker m_horseshoeTracker;
	ExportListTracker m_exportTracker;
	OysterTracker m_oysterTracker;
	std::array<CollectibleTracker*, 6> m_collectibles{
		&m_tagTracker,&m_stuntJumpTracker,&m_snapshotTracker, &m_horseshoeTracker, &m_exportTracker,
		&m_oysterTracker };

	PendingChecks<std::string> m_pendingMissions;
	PendingChecks<int> m_pendingSubmissions;
	PendingChecks<int> m_pendingSubmissionLevels;

	void resyncBaselines();

	SubmissionTracker* findTracker(int t_submissionID);
	bool missionChecker();
	bool submissionLevelChecker();
	void initializeMissionList();
	void enforceSubmissionRewards();
};

