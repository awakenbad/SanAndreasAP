#include "CheckListener.h"
#include "TagPositions.h"
#include "EntityIDs.h"
#include "SaveDataManager.h"
#include "common.h"
#include <algorithm>

namespace
{
	constexpr char TAGS_CLAIMED_KEY[] = "tags_claimed";
}

CheckListener::CheckListener() : m_pickUpCounter(CPickups::aPickUpsCollected)
{
	currentMission = CStats::LastMissionPassedName;
	lastMission = CStats::LastMissionPassedName;
	m_lastValuePickUpCounter = *m_pickUpCounter;
	initializeMissionList();

	submissionTrackers.push_back(std::make_unique<ParamedicTracker>(PARAMEDIC_ID));
	submissionTrackers.push_back(std::make_unique<VigilanteTracker>(VIGILANTE_ID));
	submissionTrackers.push_back(std::make_unique<FirefighterTracker>(FIREFIGHTER_ID));
	submissionTrackers.push_back(std::make_unique<BurglaryTracker>(BURGLARY_ID));
	submissionTrackers.push_back(std::make_unique<TaxiTracker>(TAXI_ID));
	submissionTrackers.push_back(std::make_unique<LosSantosGymTracker>(LOS_SANTOS_GYM_ID));
	submissionTrackers.push_back(std::make_unique<TruckingTracker>(TRUCKING_ID));
}


bool CheckListener::tagChecker()
{
	float currentTagCount = static_cast<float>(*reinterpret_cast<int32_t*>(TAGS_SPRAYED_ADDR));

	if (!m_tagCountInitialized)
	{
		m_lastTagCount = currentTagCount;
		m_tagCountInitialized = true;
		return m_pendingTags.hasPending();
	}

	int delta = static_cast<int>(currentTagCount) - static_cast<int>(m_lastTagCount);
	if (delta > 0)
	{
		m_lastTagCount = currentTagCount;

		CPlayerPed* player = FindPlayerPed();
		if (player)
		{
			findClosestTag(player, delta);
		}
	}

	return m_pendingTags.hasPending();
}

int CheckListener::getPendingTagIndex()
{
	if (!m_pendingTags.hasPending()) return -1;
	return m_pendingTags.front();
}

void CheckListener::confirmTagSent()
{
	m_pendingTags.confirm();
}

const std::array<bool, 100>& CheckListener::getClaimedTags() const
{
	return m_tagClaimed;
}

// TEMPORARY
std::string CheckListener::missionDebugLine() const
{
	std::string key(CStats::LastMissionPassedName);

	int id = -1;
	for (int i = 0; i < static_cast<int>(missions.size()); ++i)
	{
		if (missions[i] == key)
		{
			id = i;
			break;
		}
	}

	return "DBG LastMission: [" + key + "] id=" + std::to_string(id);
}

void CheckListener::save(SaveDataManager& t_saveData)
{
	std::string tagBits(m_tagClaimed.size(), '0');
	for (size_t i = 0; i < m_tagClaimed.size(); ++i)
	{
		if (m_tagClaimed[i]) tagBits[i] = '1';
	}
	t_saveData.setValue(TAGS_CLAIMED_KEY, tagBits);

	for (const auto& tracker : submissionTrackers)
	{
		tracker->save(t_saveData);
	}
}

void CheckListener::load(const SaveDataManager& t_saveData)
{
	// Baselines first: everything below restores what has already been checked, and detection
	// diffs live counters against these. Re-taking them after a load is what stops the jump from
	// the old session's values to the loaded save's being read as fresh progress.
	resyncBaselines();

	// A save written before this key existed (or a shorter string from an older build) leaves the
	// remaining tags unclaimed rather than reading past the end.
	std::string tagBits = t_saveData.getValue(TAGS_CLAIMED_KEY, std::string(100, '0'));
	for (size_t i = 0; i < m_tagClaimed.size(); ++i)
	{
		m_tagClaimed[i] = i < tagBits.size() && tagBits[i] == '1';
	}

	for (const auto& tracker : submissionTrackers)
	{
		tracker->load(t_saveData);
	}
}

bool CheckListener::pickUpChecker()
{
	if (m_lastValuePickUpCounter < *m_pickUpCounter)
	{
		m_lastValuePickUpCounter = *m_pickUpCounter;
		m_pendingPickUps.push(0);
	}
	return m_pendingPickUps.hasPending();
}

void CheckListener::confirmPickUpSent()
{
	m_pendingPickUps.confirm();
}

bool CheckListener::missionChecker()
{
	currentMission = CStats::LastMissionPassedName;

	if (lastMission != currentMission)
	{
		lastMission = currentMission;

		int missionIDcounter = 0;
		for (const auto& mission : missions)
		{
			if (mission == currentMission)
			{
				break;
			}
			missionIDcounter++;
		}

		if (missionIDcounter < static_cast<int>(missions.size()))
		{
			if (SubmissionTracker* st = findTracker(missionIDcounter))
			{
				st->submissionWasCompleted();
			}

			m_pendingMissions.push(currentMission);
		}
		// Unknown GXT keys fall through unqueued instead of sending CHECK:MISSION:-1.
	}
	return m_pendingMissions.hasPending();
}

void CheckListener::confirmMissionSent()
{
	m_pendingMissions.confirm();
}

void CheckListener::initializeMissionList()
{
	missions =
	{
		"INITIAL",   // 0   Initial 1
		"INITIL2",   // 1   Initial 2
		"INTRO",     // 2   Intro
		"NONE",      // 3   Video Game: They Crawled From Uranus
		"DUAL",      // 4   Video Game: Duality
		"SHTR",      // 5   Video Game: Go Go Space Monkey
		"GRAV",      // 6   Video Game: Let's Get Ready To Bumble
		"OTB",       // 7   Video Game: Inside Track Betting
		"POOL_2",    // 8   Pool
		"LOWR",      // 9   Lowrider (Bet And Dance)
		"ZERO_5",    // 10  Beefy Baron
		"INTRO_1",   // 11  Big Smoke
		"INTRO_2",   // 12  Ryder
		"SWEET_1",   // 13  Tagging Up Turf
		"SWEET1B",   // 14  Cleaning The Hood
		"SWEET_3",   // 15  Drive-Thru
		"SWEET_2",   // 16  Nines And AK's
		"SWEET_4",   // 17  Drive-By
		"SWEET_5",   // 18  Sweet's Girl
		"SWEET_6",   // 19  Cesar Vialpando
		"SWEET_7",   // 20  Los Sepulcros
		"CRASH_2",   // 21  Doberman
		"CRASH_1",   // 22  Burning Desire
		"CRASH_3",   // 23  Gray Imports
		"RYDER_1",    // 24  Home Invasion
		"RYDER_3",   // 25  Catalyst
		"RYDER_2",   // 26  Robbing Uncle Sam
		"SMOKE_1",    // 27  OG Loc
		"SMOKE_2",   // 28  Running Dog
		"SMOKE_3",   // 29  Wrong Side of the Tracks
		"SMOKE_4",   // 30  Just Business
		"STRAP_1",   // 31  Life's a Beach
		"STRAP_2",   // 32  Madd Dogg's Rhymes
		"STRAP_3",   // 33  Management Issues
		"STRAP_4",   // 34  House Party
		"CPRACE",    // 35  Race Tournament / 8-track / Dirt Track
		"CESAR_1",   // 36  Lowrider (High Stakes)
		"LA1FIN1",   // 37  Reuniting The Families
		"LA1FIN2",   // 38  The Green Sabre
		"BCRASH1",   // 39  Badlands (verified in-game)
		"CATALIN",   // 40  First Date - NEVER FIRES: cutscene-style mission, the game does not
		             //     update LastMissionPassedName for it (verified in-game). Placeholder
		             //     key kept so the ID numbering stays intact.
		"CAT_1",     // 41  Local Liquor Store (verified in-game)
		"CAT_2",     // 42  Small Town Bank (verified in-game)
		"CAT_3",     // 43  Tanker Commander (verified in-game)
		"CAT_4",     // 44  Against All Odds (verified in-game)
		"CATCUT",    // 45  King in Exile - NEVER FIRES, same as First Date (verified in-game)
		"TRUTH_1",   // 46  Body Harvest (verified in-game)
		"TRUTH_2",   // 47  Are you going to San Fierro? (verified in-game)
		"BCESAR4",   // 48  Wu Zi Mu (verified in-game; Farewell My Love is separate - see 135)
		"GAR_1",   // 49  Wear Flowers In Your Hair
		"GAR_2",     // 50  Deconstruction
		"SCRA_1",   // 51  555 WE TIP
		"SCRA_2",   // 52  Snail Trail
		"WUZI_1",    // 53  Mountain Cloud Boys
		"FAR_4",   // 54  Ran Fa Li
		"FAR_5",    // 55  Lure
		"WUZI_2",    // 56  Amphibious Assault
		"WUZI_4",    // 57  The Da Nang Thang
		"SYND_1",     // 58  Photo Opportunity
		"SYND_2",     // 59  Jizzy
		"SYND_3",     // 60  Outrider
		"SYND_4",    // 61  Ice Cold Killa
		"SYND_6",     // 62  Toreno's Last Flight
		"SYND_7",     // 63  Yay Ka-Boom-Boom
		"SYND_5",     // 64  Pier 69
		"FAR_2",    // 65  T-Bone Mendez
		"FAR_3",     // 66  Mike Toreno
		"STEAL_1",   // 67  Zeroing In
		"STEAL_2",   // 68  Test Drive
		"STEAL_4",   // 69  Customs Fast Track
		"STEAL_5",   // 70  Puncture Wounds
		"FAR_1",    // 71  Back to School
		"ZERO_1",    // 72  Air Raid
		"ZERO_2",    // 73  Supply Lines...
		"ZERO_4",    // 74  New Model Army
		"TORENO1",   // 75  Monster
		"TORENO2",   // 76  Highjack
		"DES_3",     // 77  Interdiction
		"DESERT4",   // 78  Verdant Meadows
		"DESERT6",   // 79  N.O.E.
		"DESERT9",   // 80  Stowaway
		"MAF_4",     // 81  Black Project
		"DES_10",    // 82  Green Goo
		"DESERT5",   // 83  Learning to Fly
		"CASINO1",   // 84  Fender Ketchup
		"CASINO2",   // 85  Explosive Situation
		"CASINO3",   // 86  You've Had Your Chips
		"CASINO7",   // 87  Fish in a Barrel
		"CASINO4",   // 88  Don Peyote
		"CASINO5",   // 89  Intensive Care
		"CASINO6",   // 90  The Meat Business
		"CASINO9",   // 91  Freefall
		"CASIN10",   // 92  Saint Mark's Bistro
		"VCRASH1",   // 93  Misappropriation
		"VCR_2",     // 94  High Noon
		"DOC_2",     // 95  Madd Dogg
		"HEIST_1",   // 96  Architectural Espionage
		"HEIST_3",   // 97  Key To Her Heart
		"HEIST_2",   // 98  Dam And Blast
		"HEIST_4",   // 99  Cop Wheels
		"HEIST_5",   // 100 Up, Up and Away!
		"HEIST_9",   // 101 Breaking the Bank at Caligula's
		"MANSIO1",   // 102 A Home In The Hills
		"MANSIO2",   // 103 Vertical Bird
		"MANSIO3",   // 104 Home Coming
		"MANSON5",   // 105 Cut Throat Business
		"GROVE_1",   // 106 Beat Down on B Dup
		"GROVE_2",   // 107 Grove 4 Life
		"RIOT_1",    // 108 Riot
		"RIOT_2",    // 109 Los Desperados
		"FINALEA",   // 110 End Of The Line (1)
		"FINALEB",   // 111 End Of The Line (2)
		"FINALEC",   // 112 End Of The Line (3)
		"SHRANGE",   // 113 Shooting range
		"GYMLS",     // 114 Los Santos Gym Fight School
		"GYMSF",     // 115 San Fierro Gym Fight School
		"GYMLV",     // 116 Las Venturas Gym Fight School
		"TRUCK",     // 117 Trucking
		"QUARRY",    // 118 Quarry
		"BOAT",      // 119 Boat School
		"BSKOOL",    // 120 Bike School
		"TAXIODD",   // 121 Taxi-Driver Sub-Mission
		"AMBULAN",   // 122 Paramedic Sub-Mission
		"FIRETRU",   // 123 Firefighter Sub-Mission
		"COPCAR",    // 124 Vigilante Sub-Mission
		"BURGJB",    // 125 Burglary Sub-Mission
		"FREIGHT",   // 126 Freight Train Sub-Mission
		"PIMP",      // 127 Pimping Sub-Mission
		"BLOOD",     // 128 Arena Mission: Blood Ring
		"KICKSTA",   // 129 Arena Mission: Kickstart
		"TRIA",      // 130 Beat the Cock!
		"BCOUR",     // 131 Courier
		"MTBIKER",   // 132 The Chiliad Challenge
		"STUNT",     // 133 BMX / NRG-500 STUNT Mission
		"BUYPRO1",   // 134 Buy Properties Mission
		// Appended past the original range: missions the source table
		// merged into one row but which the game reports as separate completions. Appending
		// keeps every existing index (= AP location ID) stable.
		"BCES4_2",   // 135 Farewell, My Love... (verified in-game; row 48 covers only Wu Zi Mu)
	};
}

void CheckListener::enforceSubmissionRewards()
{
	for (const auto& st : submissionTrackers)
	{
		st->enforceSubmissionReward();
	}
}

void CheckListener::findClosestTag(CPlayerPed* player, int delta)
{
	CVector playerPos = player->GetPosition();

	std::vector<std::pair<float, int>> distances;
	for (int i = 0; i < static_cast<int>(tagPositions.size()); ++i)
	{
		if (m_tagClaimed[i]) continue;
		distances.push_back({ CVector::Distance(playerPos, tagPositions[i]), i });
	}
	std::sort(distances.begin(), distances.end(),
		[](const auto& a, const auto& b) { return a.first < b.first; });

	for (int i = 0; i < delta && i < static_cast<int>(distances.size()); ++i)
	{
		int tagIndex = distances[i].second;
		m_tagClaimed[tagIndex] = true;
		m_pendingTags.push(tagIndex);
	}
}

CheckEvent CheckListener::update()
{
	enforceSubmissionRewards();

	// Baselines captured in the constructor are meaningless - nothing is loaded at process
	// start, so every polled counter reads its pre-game value. And the session's first Load
	// Game can slip past SaveDataManager's change detection entirely (the game pre-populates
	// the last-used slot name at startup, so re-loading that same slot changes nothing
	// observable). Waiting until the player actually exists and resyncing right then stops
	// the menu-to-loaded-save jump in those counters from firing phantom checks.
	if (!m_baselinesInitialized)
	{
		if (!FindPlayerPed()) return CheckEvent::None;
		resyncBaselines();
		m_baselinesInitialized = true;
	}

	CheckEvent event = CheckEvent::None;
	if (pickUpChecker())
	{
		event = CheckEvent::PickUp;
	}
	if (missionChecker())
	{
		event = CheckEvent::Mission;
	}
	if (tagChecker())
	{
		event = CheckEvent::Tag;
	}
	if (submissionLevelChecker())
	{
		event = CheckEvent::Submission;
	}
	return event;
}

bool CheckListener::submissionLevelChecker()
{
	// Tiered submissions report their own newly reached tiers; each tracker owns where its
	// progress comes from and its tier layout, so nothing here needs to know which are tiered.
	std::vector<int> newTierSlots;
	for (const auto& tracker : submissionTrackers)
	{
		tracker->pollNewTierSlots(newTierSlots);
	}
	for (int slot : newTierSlots)
	{
		m_pendingSubmissionLevels.push(slot);
	}



	if (SubmissionTracker* st = findTracker(LOS_SANTOS_GYM_ID))
	{
		if (!st->getSubmissionCompleted() && static_cast<LosSantosGymTracker*>(st)->pollCompletion())
		{
			st->submissionWasCompleted();
			m_pendingSubmissions.push(LOS_SANTOS_GYM_ID);
		}
	}

	return m_pendingSubmissions.hasPending();
}

int CheckListener::getPendingSubmissionId()
{
	if (!m_pendingSubmissions.hasPending()) return -1;
	return m_pendingSubmissions.front();
}

void CheckListener::confirmSubmissionSent()
{
	m_pendingSubmissions.confirm();
}

bool CheckListener::hasPendingSubmissionLevel() const
{
	return m_pendingSubmissionLevels.hasPending();
}

int CheckListener::getPendingSubmissionLevelSlot() const
{
	if (!m_pendingSubmissionLevels.hasPending()) return -1;
	return m_pendingSubmissionLevels.front();
}

void CheckListener::confirmSubmissionLevelSent()
{
	m_pendingSubmissionLevels.confirm();
}


std::string CheckListener::getMissionID()
{
	if (!m_pendingMissions.hasPending()) return std::to_string(NO_MISSION);

	int counter = 0;
	for (const std::string& missionName : missions)
	{
		if (m_pendingMissions.front() == missionName)
		{
			return std::to_string(counter);
		}
		counter++;
	}
	return std::to_string(NO_MISSION);
}

void CheckListener::submissionCheckWasReceived(int t_submissionID)
{
	if (SubmissionTracker* st = findTracker(t_submissionID))
	{
		st->checkWasReceived();
	}
}

SubmissionTracker* CheckListener::findTracker(int t_submissionID)
{
	for (const auto& st : submissionTrackers)
	{
		if (st->getSubmissionID() == t_submissionID) return st.get();
	}
	return nullptr;
}

// Only story missions spend a Progressive Mission. Optional side missions still send their check
// like anything else - they just cost nothing to play, so running out of Progressive Missions
// never locks the player out of them.
bool CheckListener::isStoryMission(int missionId)
{
	if (missionId == 35) return false;

	for (int optionalId : OPTIONAL_MISSION_IDS)
	{
		if (missionId == optionalId) return false;
	}

	if (missionId == 135) return true; // Farewell, My Love... - appended past the original range
	return missionId >= 11 && missionId <= 112;
}

void CheckListener::resyncBaselines()
{
	currentMission = CStats::LastMissionPassedName;
	lastMission = currentMission;

	m_lastValuePickUpCounter = *m_pickUpCounter;

	m_lastTagCount = static_cast<float>(*reinterpret_cast<int32_t*>(TAGS_SPRAYED_ADDR));
}
