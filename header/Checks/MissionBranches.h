#pragma once
#include <string>

class BranchProgress;

const std::string& branchOfMission(int t_missionId);

int branchRadarSprite(const std::string& t_branch);

const std::string& branchDisplayName(const std::string& t_branch);

const char* activeBranchAtMarker(size_t t_markerIndex, const BranchProgress& t_progress);

int missionMarkerIndexAt(float t_x, float t_y);

bool markerIsBlocked(int t_markerIndex, const BranchProgress& t_progress);

int checkIdForMission(int t_missionId, const BranchProgress& t_progress);

inline const char* const missionStartPosBranch[] = {
	nullptr,
	"Ryder",
	"Sweet",
	"OG Loc",
	"Sweet",
	"C.R.A.S.H.",
	"Big Smoke",
	"OG Loc",
	"Cesar",
	"C.R.A.S.H.",
	"Catalina",
	"Catalina",
	"The Truth",
	"The Truth",
	"Cesar",
	"Garage",
	"Woozie",
	"Triads",
	"Loco Syndicate",
	nullptr,
	nullptr,
	nullptr,
	"Toreno",
	"Toreno",
	"Toreno",
	"Four Dragons Casino",
	"Four Dragons Casino",
	"Caligula's Palace",
	"C.R.A.S.H.",
	"C.R.A.S.H.",
	"Madd Dogg",
	"Return",
	"Cesar",
	nullptr,
};

inline constexpr size_t MISSION_START_POS_BRANCH_COUNT =
sizeof(missionStartPosBranch) / sizeof(missionStartPosBranch[0]);
