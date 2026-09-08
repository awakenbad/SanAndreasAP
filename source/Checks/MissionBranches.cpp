#include "MissionBranches.h"

#include <CRadar.h>
#include <unordered_map>

#include "BranchProgress.h"
#include "EntityIDs.h"

namespace
{
	const std::unordered_map<int, std::string> MISSION_BRANCH = {
		{12, "Ryder"}, {13, "Sweet"}, {14, "Sweet"}, {15, "Sweet"}, {16, "Sweet"}, {17, "Sweet"},
		{18, "Sweet"}, {19, "Sweet"}, {20, "Sweet"}, {21, "Sweet"}, {22, "C.R.A.S.H."},
		{23, "C.R.A.S.H."}, {24, "Ryder"}, {25, "Ryder"}, {26, "Ryder"}, {27, "Big Smoke"},
		{28, "Big Smoke"}, {29, "Big Smoke"}, {30, "Big Smoke"}, {31, "OG Loc"}, {32, "OG Loc"},
		{33, "OG Loc"}, {34, "OG Loc"}, {36, "Cesar"}, {37, "Sweet"}, {38, "Sweet"},
		{39, "C.R.A.S.H."}, {41, "Catalina"}, {42, "Catalina"}, {43, "Catalina"}, {44, "Catalina"},
		{46, "The Truth"}, {47, "The Truth"}, {48, "Cesar"}, {49, "Garage"}, {50, "Garage"},
		{51, "Garage"}, {52, "C.R.A.S.H."}, {53, "Woozie"}, {54, "Woozie"}, {55, "Woozie"},
		{56, "Woozie"}, {57, "Woozie"}, {58, "Triads"}, {59, "Loco Syndicate"}, {60, "Triads"},
		{61, "Triads"}, {62, "Triads"}, {63, "Triads"}, {64, "Triads"}, {65, "Loco Syndicate"},
		{66, "Loco Syndicate"}, {75, "Toreno"}, {76, "Toreno"}, {77, "Toreno"}, {78, "Toreno"},
		{79, "Toreno"}, {80, "Toreno"}, {81, "Toreno"}, {82, "Toreno"}, {83, "Toreno"},
		{84, "Four Dragons Casino"}, {85, "Four Dragons Casino"}, {86, "Four Dragons Casino"},
		{87, "Four Dragons Casino"}, {88, "Four Dragons Casino"}, {89, "Caligula's Palace"},
		{90, "Caligula's Palace"}, {91, "Caligula's Palace"}, {92, "Caligula's Palace"},
		{93, "C.R.A.S.H."}, {94, "C.R.A.S.H."}, {95, "Madd Dogg"}, {102, "Four Dragons Casino"},
		{103, "Return"}, {104, "Return"}, {105, "Return"}, {106, "Sweet"}, {107, "Sweet"},
		{108, "Sweet"}, {109, "Sweet"}, {112, "Sweet"}, {135, "Cesar"},
	};

	const std::string NO_BRANCH;

	constexpr int FIRST_CATALINA_SLOT = 41;
	constexpr int LAST_CATALINA_SLOT = 44;

	constexpr size_t ANGEL_PINE_TRAILER_MARKER = 9;
	constexpr size_t DOHERTY_GARAGE_MARKER = 15;
	constexpr size_t MADD_DOGG_MANSION_MARKER = 31;
	constexpr size_t JOHNSON_HOUSE_MARKER = 33;

	const char* activeBranchAtAngelPineTrailer(const BranchProgress& t_progress)
	{
		if (!t_progress.missionCompleted(39)) return "C.R.A.S.H.";
		return nullptr;
	}

	const char* activeBranchAtDohertyGarage(const BranchProgress& t_progress)
	{
		if (!t_progress.missionCompleted(50)) return "Garage";
		if (!t_progress.missionCompleted(58)) return "Triads";
		if (!t_progress.missionCompleted(59)) return nullptr;
		if (!t_progress.missionCompleted(60)) return "Triads";
		if (!t_progress.missionCompleted(52)) return "C.R.A.S.H.";
		if (!t_progress.missionCompleted(63)) return "Triads";
		return nullptr;
	}

	const char* activeBranchAtMaddDoggMansion(const BranchProgress& t_progress)
	{
		if (!t_progress.missionCompleted(105)) return "Return";  // 103, 104, then 105
		if (!t_progress.missionCompleted(108)) return "Sweet";   // Riot
		return nullptr;
	}

	const char* activeBranchAtJohnsonHouse(const BranchProgress& t_progress)
	{
		if (!t_progress.missionCompleted(104)) return nullptr;   // Home Coming
		if (!t_progress.missionCompleted(106)) return "Sweet";   // Beat Down on B Dup
		return nullptr;
	}

	constexpr float MARKER_TOLERANCE_SQ = 9.0f;
}

int missionMarkerIndexAt(float t_x, float t_y)
{
	size_t count = missionStartPos.size();
	if (count > MISSION_START_POS_BRANCH_COUNT) count = MISSION_START_POS_BRANCH_COUNT;

	for (size_t i = 0; i < count; ++i)
	{
		float dx = t_x - missionStartPos[i].x;
		float dy = t_y - missionStartPos[i].y;
		if (dx * dx + dy * dy <= MARKER_TOLERANCE_SQ) return static_cast<int>(i);
	}
	return -1;
}

bool markerIsBlocked(int t_markerIndex, const BranchProgress& t_progress)
{
	const char* branch = activeBranchAtMarker(static_cast<size_t>(t_markerIndex), t_progress);
	return branch != nullptr && t_progress.isBlocked(branch);
}

int checkIdForMission(int t_missionId, const BranchProgress& t_progress)
{
	if (t_missionId < FIRST_CATALINA_SLOT || t_missionId > LAST_CATALINA_SLOT) return t_missionId;

	int slot = FIRST_CATALINA_SLOT + t_progress.completed("Catalina");
	return slot > LAST_CATALINA_SLOT ? LAST_CATALINA_SLOT : slot;
}

const std::string& branchOfMission(int t_missionId)
{
	auto it = MISSION_BRANCH.find(t_missionId);
	return it == MISSION_BRANCH.end() ? NO_BRANCH : it->second;
}

const char* activeBranchAtMarker(size_t t_markerIndex, const BranchProgress& t_progress)
{
	if (t_markerIndex == ANGEL_PINE_TRAILER_MARKER) return activeBranchAtAngelPineTrailer(t_progress);
	if (t_markerIndex == DOHERTY_GARAGE_MARKER) return activeBranchAtDohertyGarage(t_progress);
	if (t_markerIndex == MADD_DOGG_MANSION_MARKER) return activeBranchAtMaddDoggMansion(t_progress);
	if (t_markerIndex == JOHNSON_HOUSE_MARKER) return activeBranchAtJohnsonHouse(t_progress);

	return t_markerIndex < MISSION_START_POS_BRANCH_COUNT ? missionStartPosBranch[t_markerIndex]
		: nullptr;
}

int branchRadarSprite(const std::string& t_branch)
{
	static const std::unordered_map<std::string, int> BRANCH_SPRITE = {
		{"Sweet", RADAR_SPRITE_SWEET},
		{"Ryder", RADAR_SPRITE_RYDER},
		{"Big Smoke", RADAR_SPRITE_BIGSMOKE},
		{"OG Loc", RADAR_SPRITE_OGLOC},
		{"Cesar", RADAR_SPRITE_CESARVIAPANDO},
		{"C.R.A.S.H.", RADAR_SPRITE_CRASH1},
		{"Catalina", RADAR_SPRITE_CATALINAPINK},
		{"The Truth", RADAR_SPRITE_THETRUTH},
		{"Garage", RADAR_SPRITE_CJ},
		{"Triads", RADAR_SPRITE_TRIADS},
		{"Loco Syndicate", RADAR_SPRITE_LOGOSYNDICATE},
		{"Woozie", RADAR_SPRITE_WOOZIE},
		{"Toreno", RADAR_SPRITE_TORENORANCH},
		{"Four Dragons Casino", RADAR_SPRITE_TRIADSCASINO},
		{"Caligula's Palace", RADAR_SPRITE_MAFIACASINO},
		{"Madd Dogg", RADAR_SPRITE_MADDOG},
		{"Return", RADAR_SPRITE_CJ},
	};

	auto it = BRANCH_SPRITE.find(t_branch);
	return it == BRANCH_SPRITE.end() ? RADAR_SPRITE_CJ : it->second;
}

const std::string& branchDisplayName(const std::string& t_branch)
{
	static const std::string GARAGE_AS_CJ = "CJ";
	if (t_branch == "Garage") return GARAGE_AS_CJ;
	return t_branch;
}
