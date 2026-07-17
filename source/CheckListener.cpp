#include "CheckListener.h"
#include "TagPositions.h"
#include "EntityIDs.h"
#include "common.h"
#include <algorithm>

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

void CheckListener::restoreClaimedTags(const std::array<bool, 100>& t_claimed)
{
	m_tagClaimed = t_claimed;
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
		"HOODS_5",   // 18  Sweet's Girl
		"SWEET_6",   // 19  Cesar Vialpando
		"SWEET_7",   // 20  Los Sepulcros
		"CRASH_4",   // 21  Doberman
		"CRASH_1",   // 22  Burning Desire
		"DRUGS_3",   // 23  Gray Imports
		"GUNS_1",    // 24  Home Invasion
		"RYDER_3",   // 25  Catalyst
		"RYDER_2",   // 26  Robbing Uncle Sam
		"TWAR_7",    // 27  OG Loc
		"SMOKE_2",   // 28  Running Dog
		"SMOKE_3",   // 29  Wrong Side of the Tracks
		"DRUGS_1",   // 30  Just Business
		"MUSIC_1",   // 31  Life's a Beach
		"MUSIC_2",   // 32  Madd Dogg's Rhymes
		"MUSIC_3",   // 33  Management Issues
		"MUSIC_5",   // 34  House Party
		"CPRACE",    // 35  Race Tournament / 8-track / Dirt Track
		"CESAR_1",   // 36  Lowrider (High Stakes)
		"DRUGS_4",   // 37  Reuniting The Families
		"LA1FIN2",   // 38  The Green Sabre
		"BCRASH1",   // 39  Badlands
		"CATALIN",   // 40  First Date
		"CAT_1",     // 41  Local Liquor Store
		"CAT_2",     // 42  Small Town Bank
		"CAT_3",     // 43  Tanker Commander
		"CAT_4",     // 44  Against All Odds
		"CATCUT",    // 45  King in Exile
		"TRUTH_1",   // 46  Body Harvest
		"TRUTH_2",   // 47  Are you going to San Fierro?
		"BCESAR4",   // 48  Wu Zi Mu / Farewell, My Love...
		"GARAG_1",   // 49  Wear Flowers In Your Hair
		"DECON",     // 50  Deconstruction
		"SCRASH3",   // 51  555 WE TIP
		"SCRASH2",   // 52  Snail Trail
		"WUZI_1",    // 53  Mountain Cloud Boys
		"FARLIE4",   // 54  Ran Fa Li
		"DRIV_6",    // 55  Lure
		"WUZI_2",    // 56  Amphibious Assault
		"WUZI_5",    // 57  The Da Nang Thang
		"SYN_1",     // 58  Photo Opportunity
		"SYN_2",     // 59  Jizzy
		"SYN_3",     // 60  Outrider
		"SYND_4",    // 61  Ice Cold Killa
		"SYN_6",     // 62  Toreno's Last Flight
		"SYN_7",     // 63  Yay Ka-Boom-Boom
		"SYN_5",     // 64  Pier 69
		"DRIV_2",    // 65  T-Bone Mendez
		"DRIV_3",    // 66  Mike Toreno
		"STEAL_1",   // 67  Zeroing In
		"STEAL_2",   // 68  Test Drive
		"STEAL_4",   // 69  Customs Fast Track
		"STEAL_5",   // 70  Puncture Wounds
		"DSKOOL",    // 71  Back to School
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
	const std::vector<std::pair<int, unsigned short>> submissionIdToLevelStat = {
		{ PARAMEDIC_ID, STAT_HIGHEST_PARAMEDIC_MISSION_LEVEL },
		{ FIREFIGHTER_ID, STAT_HIGHEST_FIREFIGHTER_MISSION_LEVEL },
		{ VIGILANTE_ID, STAT_HIGHEST_VIGILANTE_MISSION_LEVEL },
	};

	for (const auto& [submissionId, stat] : submissionIdToLevelStat)
	{
		if (CStats::GetStatValue(stat) < 12.0f) continue;

		SubmissionTracker* st = findTracker(submissionId);
		if (st && !st->getSubmissionCompleted())
		{
			st->submissionWasCompleted();
			m_pendingSubmissions.push(submissionId);
		}
	}

	if (*reinterpret_cast<int32_t*>(TAXI_FARES_ADDR) >= TAXI_FARES_FOR_COMPLETION)
	{
		SubmissionTracker* st = findTracker(TAXI_ID);
		if (st && !st->getSubmissionCompleted())
		{
			st->submissionWasCompleted();
			m_pendingSubmissions.push(TAXI_ID);
		}
	}

	if (CStats::GetStatValue(STAT_MONEY_MADE_FROM_BURGLARY) >= BURGLARY_LOOT_FOR_COMPLETION)
	{
		SubmissionTracker* st = findTracker(BURGLARY_ID);
		if (st && !st->getSubmissionCompleted())
		{
			st->submissionWasCompleted();
			m_pendingSubmissions.push(BURGLARY_ID);
		}
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

bool CheckListener::isStoryMission(int missionId)
{
	if (missionId == 35) return false;
	return missionId >= 11 && missionId <= 112;
}

const std::vector<std::unique_ptr<SubmissionTracker>>& CheckListener::getSubmissionTrackers() const
{
	return submissionTrackers;
}

void CheckListener::resyncBaselines()
{
	currentMission = CStats::LastMissionPassedName;
	lastMission = currentMission;

	m_lastValuePickUpCounter = *m_pickUpCounter;

	m_lastTagCount = static_cast<float>(*reinterpret_cast<int32_t*>(TAGS_SPRAYED_ADDR));
}
