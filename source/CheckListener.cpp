#include "CheckListener.h"


CheckListener::CheckListener() : m_pickUpCounter(CPickups::aPickUpsCollected)
{
	currentMission = CStats::LastMissionPassedName;
	lastMission = CStats::LastMissionPassedName;
	m_lastValuePickUpCounter = *m_pickUpCounter;
	initializeMissionList();
}

bool CheckListener::tagChecker()
{
	return false;
}

bool CheckListener::pickUpChecker()
{
	if (m_lastValuePickUpCounter < *m_pickUpCounter)
	{
		CMessages::AddMessageJumpQ("Picked up an item", 100, 0);
		m_lastValuePickUpCounter = *m_pickUpCounter;
		return true;
	}
	return false;
}

bool CheckListener::missionChecker()
{
	currentMission = CStats::LastMissionPassedName;
	enforceSubmissionRewards();
	if (lastMission != currentMission)
	{
		if (currentMission == missions[PARAMEDIC_ID])
		{
			paramedicCompleted = true;
		}
		if (currentMission == missions[VIGILANTE_ID])
		{
			vigilanteCompleted = true;
		}

		lastMission = currentMission;
		return true;
	}
	return false;
}

void CheckListener::initializeMissionList()
{
	missions =
	{
		"INITIAL",   // 0   Initial 1
		"INITIL_2",  // 1   Initial 2
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
		"LA1FIN_2",  // 38  The Green Sabre
		"BCRASH_1",  // 39  Badlands
		"CATALIN",   // 40  First Date
		"CAT_1",     // 41  Local Liquor Store
		"CAT_2",     // 42  Small Town Bank
		"CAT_3",     // 43  Tanker Commander
		"CAT_4",     // 44  Against All Odds
		"CATCUT",    // 45  King in Exile
		"TRUTH_1",   // 46  Body Harvest
		"TRUTH_2",   // 47  Are you going to San Fierro?
		"BCESAR_4",  // 48  Wu Zi Mu / Farewell, My Love...
		"GARAG_1",   // 49  Wear Flowers In Your Hair
		"DECON",     // 50  Deconstruction
		"SCRASH_3",  // 51  555 WE TIP
		"SCRASH_2",  // 52  Snail Trail
		"WUZI_1",    // 53  Mountain Cloud Boys
		"FARLIE_4",  // 54  Ran Fa Li
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
		"TORENO_1",  // 75  Monster
		"TORENO_2",  // 76  Highjack
		"DES_3",     // 77  Interdiction
		"DESERT_4",  // 78  Verdant Meadows
		"DESERT_6",  // 79  N.O.E.
		"DESERT_9",  // 80  Stowaway
		"MAF_4",     // 81  Black Project
		"DES_10",    // 82  Green Goo
		"DESERT_5",  // 83  Learning to Fly
		"CASINO_1",  // 84  Fender Ketchup
		"CASINO_2",  // 85  Explosive Situation
		"CASINO_3",  // 86  You've Had Your Chips
		"CASINO_7",  // 87  Fish in a Barrel
		"CASINO_4",  // 88  Don Peyote
		"CASINO_5",  // 89  Intensive Care
		"CASINO_6",  // 90  The Meat Business
		"CASINO_9",  // 91  Freefall
		"CASIN_10",  // 92  Saint Mark's Bistro
		"VCRASH_1",  // 93  Misappropriation
		"VCR_2",     // 94  High Noon
		"DOC_2",     // 95  Madd Dogg
		"HEIST_1",   // 96  Architectural Espionage
		"HEIST_3",   // 97  Key To Her Heart
		"HEIST_2",   // 98  Dam And Blast
		"HEIST_4",   // 99  Cop Wheels
		"HEIST_5",   // 100 Up, Up and Away!
		"HEIST_9",   // 101 Breaking the Bank at Caligula's
		"MANSIO_1",  // 102 A Home In The Hills
		"MANSIO_2",  // 103 Vertical Bird
		"MANSIO_3",  // 104 Home Coming
		"MANSON_5",  // 105 Cut Throat Business
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
		"BUYPRO_1",  // 134 Buy Properties Mission
	};
}

void CheckListener::enforceSubmissionRewards()
{
	if (healthCheckReceived && !paramedicCompleted)
	{
		CWorld::Players[0].m_nMaxHealth = 176;
	}
	if (!healthCheckReceived && paramedicCompleted)
	{
		CWorld::Players[0].m_nMaxHealth = 100;
	}

	if (armourCheckReceived && !vigilanteCompleted)	
	{
		CWorld::Players[0].m_nMaxArmour = 150;
	}
	if (!armourCheckReceived && vigilanteCompleted)
	{
		CWorld::Players[0].m_nMaxArmour = 100;
	}

	if (fireCheckReceived && !firefighterCompleted)
	{
		CWorld::Players[0].m_bFireProof = true;
	}
	if (!fireCheckReceived && firefighterCompleted)
	{
		CWorld::Players[0].m_bFireProof = false;
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
		if (currentMission == missionName)
		{
			return std::to_string(counter);
		}
		counter++;
	}
	return std::to_string(NO_MISSION);
}

void CheckListener::healthCheckWasReceived()
{
	healthCheckReceived = true;
}

void CheckListener::armourCheckWasReceived()
{
	armourCheckReceived = true;
}

void CheckListener::fireCheckWasReceived()
{
	fireCheckReceived = true;
}
