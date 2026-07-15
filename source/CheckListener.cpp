#include "CheckListener.h"


CheckListener::CheckListener() : m_pickUpCounter(CPickups::aPickUpsCollected)
{
	currentMission = CStats::LastMissionPassedName;
	lastMission = CStats::LastMissionPassedName;
	m_lastValuePickUpCounter = *m_pickUpCounter;
	initializeMissionList();

	submissionTrackers.push_back(new ParamedicTracker(PARAMEDIC_ID));
	submissionTrackers.push_back(new VigilanteTracker(VIGILANTE_ID));
	submissionTrackers.push_back(new FirefighterTracker(FIREFIGHTER_ID));
	submissionTrackers.push_back(new BurglaryTracker(BURGLARY_ID));
	submissionTrackers.push_back(new TaxiTracker(TAXI_ID));
	submissionTrackers.push_back(new LosSantosGymTracker(LOS_SANTOS_GYM_ID));
}

bool CheckListener::tagChecker()
{
	return false;
}

bool CheckListener::pickUpChecker()
{
	if (!m_pickUpEventPending && m_lastValuePickUpCounter < *m_pickUpCounter)
	{
		CMessages::AddMessageJumpQ("Picked up an item", 100, 0);
		m_pickUpEventPending = true;
	}
	return m_pickUpEventPending;
}

void CheckListener::confirmPickUpSent()
{
	m_lastValuePickUpCounter = *m_pickUpCounter;
	m_pickUpEventPending = false;
}

bool CheckListener::missionChecker()
{
	currentMission = CStats::LastMissionPassedName;
	enforceSubmissionRewards();

	if (!m_missionEventPending && lastMission != currentMission)
	{
		int missionIDcounter = 0;
		for (auto mission : missions)
		{
			if (mission == currentMission)
			{
				break;
			}
			missionIDcounter++;
		}

		for (auto st : submissionTrackers)
		{
			if (st->getSubmissionID() == missionIDcounter)
			{
				st->submissionWasCompleted();
			}
		}

		m_pendingMissionName = currentMission;
		m_missionEventPending = true;
	}
	return m_missionEventPending;
}

void CheckListener::confirmMissionSent()
{
	lastMission = m_pendingMissionName;
	m_missionEventPending = false;
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
	for (auto st : submissionTrackers)
	{
		st->enforceSubmissionReward();
	}
}

void CheckListener::spawnPickup()
{
	CPickups::GenerateNewOne(CVector{ 2493.5, -1671.0, 13.3 }, 346, 2, 100, 1, false, new char('h'));
}

CheckEvent CheckListener::update()
{
	enforceSubmissionRewards();
	CheckEvent event = CheckEvent::None;
	if (pickUpChecker())
	{
		event = CheckEvent::PickUp;
	}
	if (missionChecker())
	{
		event = CheckEvent::Mission;
	}
	return event;
}

std::string CheckListener::getMissionID()
{
	int counter = 0;
	for (std::string missionName : missions)
	{
		if (m_pendingMissionName == missionName)
		{
			return std::to_string(counter);
		}
		counter++;
	}
	return std::to_string(NO_MISSION);
}

void CheckListener::submissionCheckWasReceived(int t_submissionID)
{
	for (auto st : submissionTrackers)
	{
		if (st->getSubmissionID() == t_submissionID)
		{
			st->checkWasReceived();
		}
	}
}

bool CheckListener::isStoryMission(int missionId)
{
	if (missionId == 35) return false;
	return missionId >= 11 && missionId <= 112;
}
