#include "CheckListener.h"

#include <CCutsceneMgr.h>
#include <CPed.h>
#include <CStats.h>
#include <CZone.h>
#include <common.h>
#include <cstring>
#include <map>

#include "APProtocol.h"
#include "BikeSchoolTracker.h"
#include "BoatSchoolTracker.h"
#include "BurglaryTracker.h"
#include "Collectible.h"
#include "CourierTracker.h"
#include "DrivingSchoolTracker.h"
#include "EntityIDs.h"
#include "FirefighterTracker.h"
#include "GangTerritoryTracker.h"
#include "GlobalFlagTracker.h"
#include "GymTracker.h"
#include "ItemEffects.h"
#include "ParamedicTracker.h"
#include "ParseUtils.h"
#include "PendingChecks.h"
#include "PimpingTracker.h"
#include "QuarryTracker.h"
#include "RaceTracker.h"
#include "SaveDataManager.h"
#include "SchoolTracker.h"
#include "ShootingRangeTracker.h"
#include "SubmissionStartBlocked.h"
#include "TaxiTracker.h"
#include "TrainTracker.h"
#include "TruckingTracker.h"
#include "ValetTracker.h"
#include "VigilanteTracker.h"


CheckListener::CheckListener()
{
	currentMission = CStats::LastMissionPassedName;
	lastMission = CStats::LastMissionPassedName;
	initializeMissionList();

	submissionTrackers.push_back(std::make_unique<ParamedicTracker>(PARAMEDIC_ID));
	submissionTrackers.push_back(std::make_unique<VigilanteTracker>(VIGILANTE_ID));
	submissionTrackers.push_back(std::make_unique<FirefighterTracker>(FIREFIGHTER_ID));
	submissionTrackers.push_back(std::make_unique<BurglaryTracker>(BURGLARY_ID));
	submissionTrackers.push_back(std::make_unique<TaxiTracker>(TAXI_ID));
	submissionTrackers.push_back(std::make_unique<GymTracker>(LOS_SANTOS_GYM_ID, STYLE_BOXING, "gymls", m_styleArbiter));
	submissionTrackers.push_back(std::make_unique<GymTracker>(SAN_FIERRO_GYM_ID, STYLE_KUNG_FU, "gymsf", m_styleArbiter));
	submissionTrackers.push_back(std::make_unique<GymTracker>(LAS_VENTURAS_GYM_ID, STYLE_KNEE_HEAD, "gymlv", m_styleArbiter));
	submissionTrackers.push_back(std::make_unique<TruckingTracker>(TRUCKING_ID));
	submissionTrackers.push_back(std::make_unique<ValetTracker>(VALET_ID));
	submissionTrackers.push_back(std::make_unique<DrivingSchoolTracker>(DRIVING_SCHOOL_ID));
	submissionTrackers.push_back(std::make_unique<SchoolTracker>(FLYING_SCHOOL_ID, FLYING_SCHOOL_TIERS, FLYING_SCHOOL_SCORE_GLOBALS));
	submissionTrackers.push_back(std::make_unique<BoatSchoolTracker>(BOAT_SCHOOL_ID));
	submissionTrackers.push_back(std::make_unique<BikeSchoolTracker>(BIKE_SCHOOL_ID));
	submissionTrackers.push_back(std::make_unique<PimpingTracker>(PIMPING_ID));
	submissionTrackers.push_back(std::make_unique<QuarryTracker>(QUARRY_ID));
	submissionTrackers.push_back(std::make_unique<GangTerritoryTracker>(GANG_TERRITORY_ID));
	submissionTrackers.push_back(std::make_unique<GlobalFlagTracker>(CHILIAD_CHALLENGE_ID, CHILIAD_PASSED_GLOBAL));
	submissionTrackers.push_back(std::make_unique<GlobalFlagTracker>(BLOOD_RING_ID, BLOOD_RING_PASSED_GLOBAL));
	submissionTrackers.push_back(std::make_unique<GlobalFlagTracker>(KICKSTART_ID, KICKSTART_PASSED_GLOBAL));
	submissionTrackers.push_back(std::make_unique<CourierTracker>(COURIER_LS_ID, COURIER_LS_TIERS, LEVEL_NAME_LOS_SANTOS, COURIER_LS_PASSED_GLOBAL));
	submissionTrackers.push_back(std::make_unique<CourierTracker>(COURIER_SF_ID, COURIER_SF_TIERS, LEVEL_NAME_SAN_FIERRO, COURIER_SF_PASSED_GLOBAL));
	submissionTrackers.push_back(std::make_unique<CourierTracker>(COURIER_LV_ID, COURIER_LV_TIERS, LEVEL_NAME_LAS_VENTURAS, COURIER_LV_PASSED_GLOBAL));
	submissionTrackers.push_back(std::make_unique<RaceTracker>(RACES_LS_ID, RACES_LS_TIERS, 0));
	submissionTrackers.push_back(std::make_unique<RaceTracker>(RACES_SF_ID, RACES_SF_TIERS, 9));
	submissionTrackers.push_back(std::make_unique<RaceTracker>(RACES_LV_ID, RACES_LV_TIERS, 15));
	submissionTrackers.push_back(std::make_unique<ShootingRangeTracker>(SHOOTING_RANGE_PISTOL_ID, SHOOTING_RANGE_PISTOL_TIERS, 0));
	submissionTrackers.push_back(std::make_unique<ShootingRangeTracker>(SHOOTING_RANGE_UZI_ID, SHOOTING_RANGE_UZI_TIERS, 1));
	submissionTrackers.push_back(std::make_unique<ShootingRangeTracker>(SHOOTING_RANGE_SHOTGUN_ID, SHOOTING_RANGE_SHOTGUN_TIERS, 2));
	submissionTrackers.push_back(std::make_unique<ShootingRangeTracker>(SHOOTING_RANGE_AK47_ID, SHOOTING_RANGE_AK47_TIERS, 3));
	submissionTrackers.push_back(std::make_unique<TrainTracker>(TRAIN_ID));
	submissionTrackers.push_back(std::make_unique<GlobalFlagTracker>(COMPLETION_ID, COMPLETION_AWARDED_GLOBAL));
}

void CheckListener::locateCollectible(const std::string& t_type, int t_index)
{
	for (CollectibleTracker* collectible : m_collectibles)
	{
		if (t_type == collectible->checkType()) collectible->setLocated(t_index);
	}
}

void CheckListener::setIncludedCollectibles(const std::string& t_config)
{
	std::map<std::string, std::vector<int>> byType;

	for (const std::string& entry : APProtocol::splitList(t_config, ';'))
	{
		size_t equals = entry.find('=');
		if (equals == std::string::npos) continue;

		std::vector<int>& indices = byType[entry.substr(0, equals)];
		for (const std::string& index : APProtocol::splitList(entry.substr(equals + 1), ','))
		{
			indices.push_back(parseIntOr(index, -1));
		}
	}

	for (CollectibleTracker* collectible : m_collectibles)
	{
		auto it = byType.find(collectible->checkType());
		collectible->setIncluded(it == byType.end() ? std::vector<int>{} : it->second);
	}
}

void CheckListener::save(SaveDataManager& t_saveData)
{
	for (const CollectibleTracker* collectible : m_collectibles)
	{
		collectible->save(t_saveData);
	}

	for (const auto& tracker : submissionTrackers)
	{
		tracker->save(t_saveData);
	}
}

void CheckListener::load(const SaveDataManager& t_saveData)
{
	resyncBaselines();
	m_endingFired = false;

	for (CollectibleTracker* collectible : m_collectibles)
	{
		collectible->load(t_saveData);
	}

	for (const auto& tracker : submissionTrackers)
	{
		tracker->load(t_saveData);
	}
}

bool CheckListener::isEndingCutscenePlaying() const
{
	if (!CCutsceneMgr::ms_running) return false;

	const char* name = CCutsceneMgr::ms_cutsceneName;
	return name && std::strncmp(name, ENDING_CUTSCENE_NAME, 8) == 0;
}

bool CheckListener::missionChecker()
{
	currentMission = CStats::LastMissionPassedName;

	if (!m_endingFired && isEndingCutscenePlaying()
		&& END_OF_THE_LINE_ID < static_cast<int>(missions.size()))
	{
		m_endingFired = true;
		m_pendingMissions.push(missions[END_OF_THE_LINE_ID]);
	}

	if (lastMission != currentMission)
	{
		lastMission = currentMission;

		bool alreadySent = m_endingFired && END_OF_THE_LINE_ID < static_cast<int>(missions.size())
			&& currentMission == missions[END_OF_THE_LINE_ID];

		int missionIDcounter = 0;
		for (const auto& mission : missions)
		{
			if (mission == currentMission)
			{
				break;
			}
			missionIDcounter++;
		}

		if (!alreadySent && missionIDcounter < static_cast<int>(missions.size()))
		{
			if (SubmissionTracker* st = findTracker(missionIDcounter))
			{
				st->submissionWasCompleted();
			}

			m_pendingMissions.push(currentMission);
		}
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
		"RYDER_1",   // 24  Home Invasion
		"RYDER_3",   // 25  Catalyst
		"RYDER_2",   // 26  Robbing Uncle Sam
		"SMOKE_1",   // 27  OG Loc
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
		"CATALIN",   // 40  First Date - NEVER FIRES
		"CAT_1",     // 41  Local Liquor Store (verified in-game)
		"CAT_2",     // 42  Small Town Bank (verified in-game)
		"CAT_3",     // 43  Tanker Commander (verified in-game)
		"CAT_4",     // 44  Against All Odds (verified in-game)
		"CATCUT",    // 45  King in Exile - NEVER FIRES, same as First Date (verified in-game)
		"TRUTH_1",   // 46  Body Harvest (verified in-game)
		"TRUTH_2",   // 47  Are you going to San Fierro? (verified in-game)
		"BCESAR4",   // 48  Wu Zi Mu (verified in-game; Farewell My Love is separate - see 135)
		"GAR_1",     // 49  Wear Flowers In Your Hair
		"GAR_2",     // 50  Deconstruction
		"SCRA_1",    // 51  555 WE TIP
		"SCRA_2",    // 52  Snail Trail
		"WUZI_1",    // 53  Mountain Cloud Boys
		"FAR_4",     // 54  Ran Fa Li
		"FAR_5",     // 55  Lure
		"WUZI_2",    // 56  Amphibious Assault
		"WUZI_4",    // 57  The Da Nang Thang
		"SYND_1",    // 58  Photo Opportunity
		"SYND_2",    // 59  Jizzy
		"SYND_3",    // 60  Outrider
		"SYND_4",    // 61  Ice Cold Killa
		"SYND_6",    // 62  Toreno's Last Flight
		"SYND_7",    // 63  Yay Ka-Boom-Boom
		"SYND_5",    // 64  Pier 69
		"FAR_2",     // 65  T-Bone Mendez
		"FAR_3",     // 66  Mike Toreno
		"STEAL_1",   // 67  Zeroing In
		"STEAL_2",   // 68  Test Drive
		"STEAL_4",   // 69  Customs Fast Track
		"STEAL_5",   // 70  Puncture Wounds
		"FAR_1",     // 71  Back to School
		"ZERO_1",    // 72  Air Raid
		"ZERO_2",    // 73  Supply Lines...
		"ZERO_4",    // 74  New Model Army
		"DESERT1",   // 75  Monster
		"DESERT2",   // 76  Highjack
		"DESERT3",   // 77  Interdiction
		"DESERT4",   // 78  Verdant Meadows
		"DESERT6",   // 79  N.O.E.
		"DESERT9",   // 80  Stowaway
		"DESERT8",   // 81  Black Project
		"DESER10",   // 82  Green Goo
		"DESERT5",   // 83  Learning to Fly
		"CASINO1",   // 84  Fender Ketchup
		"CASEEN2",   // 85  Explosive Situation
		"CASINO3",   // 86  You've Had Your Chips
		"CASINO7",   // 87  Fish in a Barrel
		"CASINO4",   // 88  Don Peyote
		"CASINO5",   // 89  Intensive Care
		"CASINO6",   // 90  The Meat Business
		"CASINO9",   // 91  Freefall
		"CASIN10",   // 92  Saint Mark's Bistro
		"VCRASH1",   // 93  Misappropriation
		"VCRASH2",   // 94  High Noon
		"DOC_2",     // 95  Madd Dogg
		"HEIST_1",   // 96  Architectural Espionage
		"HEIST_3",   // 97  Key To Her Heart
		"HEIST_2",   // 98  Dam And Blast
		"HEIST_4",   // 99  Cop Wheels
		"HEIST_5",   // 100 Up, Up and Away!
		"HEIST_9",   // 101 Breaking the Bank at Caligula's
		"MAN_1",     // 102 A Home In The Hills
		"MAN_2",   // 103 Vertical Bird
		"MAN_3",   // 104 Home Coming
		"MAN_5",   // 105 Cut Throat Business
		"GROVE_1",   // 106 Beat Down on B Dup
		"GROVE_2",   // 107 Grove 4 Life
		"RIOT_1",    // 108 Riot
		"RIOT_2",    // 109 Los Desperados
		"FINALEA",   // 110 End Of The Line (1)
		"FINALEB",   // 111 End Of The Line (2)
		"RIOT_4",   // 112 End Of The Line (3)
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
		"BMX",       // 133 BMX Challenge
		"BUYPRO1",   // 134 Buy Properties Mission
		"BCES4_2",   // 135 Farewell, My Love... (verified in-game; row 48 covers only Wu Zi Mu)
		"NRG500",    // 136 NRG-500 Challenge
		"-",         // 137 Driving School
		"-",         // 138 Flying School
		"-",         // 139 Gang Territories
		"STAD_03",   // 140 8-Track
		"STAD_01",   // 141 Dirt Track
	};
}

void CheckListener::enforceSubmissionRewards()
{
	for (const auto& st : submissionTrackers)
	{
		st->enforceSubmissionReward();
	}
}

CheckEvent CheckListener::update()
{
	SubmissionStartBlocked::update(submissionTrackers);

	if (!m_baselinesInitialized)
	{
		if (!FindPlayerPed()) return CheckEvent::None;
		resyncBaselines();
		m_baselinesInitialized = true;
	}

	CheckEvent event = CheckEvent::None;
	if (missionChecker())
	{
		event = CheckEvent::Mission;
	}
	for (CollectibleTracker* collectible : m_collectibles)
	{
		collectible->update();
	}
	if (submissionLevelChecker())
	{
		event = CheckEvent::Submission;
	}

	enforceSubmissionRewards();
	return event;
}

bool CheckListener::submissionLevelChecker()
{
	std::vector<int> newTierSlots;
	for (const auto& tracker : submissionTrackers)
	{
		tracker->pollNewTierSlots(newTierSlots);
	}
	for (int slot : newTierSlots)
	{
		m_pendingSubmissionLevels.push(slot);
	}

	for (const auto& tracker : submissionTrackers)
	{
		if (!tracker->getSubmissionCompleted() && tracker->pollCompletion())
		{
			tracker->submissionWasCompleted();
			m_pendingSubmissions.push(tracker->getSubmissionID());
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

void CheckListener::collectibleUnlockWasReceived(const std::string& t_checkType)
{
	for (CollectibleTracker* collectible : m_collectibles)
	{
		if (t_checkType != collectible->checkType()) continue;

		collectible->unlock();
		return;
	}
}

void CheckListener::setGatedContent(const std::string& t_effectNames)
{
	for (CollectibleTracker* collectible : m_collectibles)
	{
		collectible->setGated(false);
	}
	for (const auto& tracker : submissionTrackers)
	{
		tracker->setGated(false);
	}

	for (const std::string& name : APProtocol::splitList(t_effectNames, ';'))
	{
		const ItemEffectSpec* spec = findItemEffect(name);
		if (!spec) continue;

		if (spec->effect == ItemEffect::SubmissionUnlock)
		{
			if (SubmissionTracker* st = findTracker(spec->submissionId)) st->setGated(true);
		}
		else if (spec->effect == ItemEffect::CollectibleUnlock && spec->trapName)
		{
			for (CollectibleTracker* collectible : m_collectibles)
			{
				if (std::strcmp(spec->trapName, collectible->checkType()) == 0) collectible->setGated(true);
			}
		}
	}
}

void CheckListener::submissionUnlockWasReceived(int t_submissionID)
{
	if (SubmissionTracker* st = findTracker(t_submissionID))
	{
		st->unlock();
	}
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

	for (int optionalId : OPTIONAL_MISSION_IDS)
	{
		if (missionId == optionalId) return false;
	}

	if (missionId == 110 || missionId == 111) return false;

	if (missionId == 135) return true; // Farewell, My Love...
	return missionId >= 11 && missionId <= 112;
}

void CheckListener::resyncBaselines()
{
	currentMission = CStats::LastMissionPassedName;
	lastMission = currentMission;

	for (CollectibleTracker* collectible : m_collectibles)
	{
		collectible->resyncBaseline();
	}
}
