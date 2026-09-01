#include "Mod.h"
#include "ModSettings.h"
#include "PlayerControl.h"
#include "APProtocol.h"
#include "ItemEffects.h"
#include "MissionBranches.h"
#include "MenuMap.h"
#include "SaveRedirect.h"
#include "MenuGate.h"
#include "StartingSaves.h"
#include "CStreaming.h"
#include "CPools.h"
#include "CPickups.h"
#include <CRadar.h>
#include <CTimer.h>
#include <CFont.h>
#include <CRGBA.h>
#include <CMenuManager.h>
#include "TagSprayBlocker.h"

Mod::Mod()
{
	ModSettings::load();

	m_apSocket.connectToServer(APProtocol::CLIENT_HOST, APProtocol::CLIENT_PORT);

	m_persistentSubsystems = { &m_checkListener, &m_branchProgress, &m_blipManager, &m_receivedItemLog, &m_trapHandler };

	GameStorageHook::setBeforeSaveCallback([this]
	{
		for (PersistentState* subsystem : m_persistentSubsystems)
		{
			subsystem->save(m_saveDataManager);
		}
		m_saveDataManager.recordValuesAtSave();
	});
}

void Mod::start()
{
    MissionLocateBlocked::install(m_branchProgress);
    BlockedMarkerTint::install(m_branchProgress);
    ShopMenuText::install(m_ammuNationShop);
    SubmissionResumeLevel::install();
    TagSprayBlocker::install();
    StreetRaceUnlock::update(m_streetRacesUnlocked);
    StreetRaceUnlock::updateDrivingSchoolBlip();
    WangCarsUnlock::update(m_wangCarsUnlocked);

    m_apSocket.update();
    pollDeathLink();

    bool loadHooked = GameStorageHook::consumeLoadHappened();

    if (GameStorageHook::consumeNewGameHappened())
    {
        resetForNewGame();
    }
    else if (loadHooked)
    {
        m_newGameRegrantPending = false;
    }

    bool worldWiped = updateWorldState(loadHooked);
    persistAndRestoreState(worldWiped, loadHooked);

    CheckEvent event = m_checkListener.update();
    applyRespawnHealthTopUp();

    sendChecksToAP(event);
    updateGameplaySystems();
    WaypointTeleport::update();

    parseIncomingMessages();
}

void Mod::pollDeathLink()
{
    if (m_deathLinkHandler.update())
    {
        m_apSocket.sendToServer(APProtocol::playerDied());
    }
}

bool Mod::updateWorldState(bool t_loadHooked)
{
    if (t_loadHooked)
    {
        m_blipManager.onWorldWiped();
    }
    return m_blipManager.render(collectBlipTargets());
}

std::vector<BlipTarget> Mod::collectBlipTargets()
{
    std::vector<BlipTarget> targets;
    for (CollectibleTracker* collectible : m_checkListener.getCollectibles())
    {
        collectible->appendBlipTargets(targets);
    }
    if (CPlayerPed* player = FindPlayerPed())
    {
        rankByDistance(targets, player->GetPosition());
    }
    return targets;
}

void Mod::applyRespawnHealthTopUp()
{
    if (!m_deathLinkHandler.consumeRespawn()) return;

    if (CPlayerPed* player = FindPlayerPed())
    {
        player->m_fHealth = static_cast<float>(CWorld::Players[0].m_nMaxHealth);
    }
}

void Mod::updateGameplaySystems()
{
    m_ammuNationShop.update();
    m_trapHandler.update();
    m_checkGiver.update();
    CityUnlock::update();
    if (ModSettings::fastTravelEnabled()) FastTravel::update();
    //BranchControllers::update(m_branchProgress);

    if (m_autoSaveManager.update())
    {
        m_notificationOverlay.showAboveRadar("Archipelago: Autosaved (slot 8)");
    }

    for (CollectibleTracker* collectible : m_checkListener.getCollectibles())
    {
        if (const char* notice = collectible->consumeLockedNotice())
        {
            m_notificationOverlay.show(notice);
        }
    }

    int purchasedSlot = m_ammuNationShop.pollPurchasedSlot();
    if (purchasedSlot >= 0)
    {
        m_pendingShopChecks.push(purchasedSlot);
        m_notificationOverlay.show("Archipelago: Checked Ammu-Nation (" + std::string(shopItems[purchasedSlot].displayName) + ")");
    }
}

static std::string collectibleLabel(const std::string& t_type)
{
    if (t_type == "TAG") return "LS Tag";
    if (t_type == "SNAPSHOT") return "SF Snapshot";
    if (t_type == "HORSESHOE") return "LV Horseshoe";
    if (t_type == "OYSTER") return "Oyster";
    return t_type;
}

void Mod::sendChecksToAP(CheckEvent t_event)
{
    switch (t_event)
    {
    case CheckEvent::Mission:
    {
        std::string missionIDStr = std::to_string(
            checkIdForMission(parseIntOr(m_checkListener.getMissionID(), -1), m_branchProgress));
        if (m_apSocket.sendToServer(APProtocol::missionCheck(missionIDStr)))
        {
            int missionID = parseIntOr(missionIDStr, -1);
            if (m_checkListener.isStoryMission(missionID))
            {
                m_branchProgress.completeMission(branchOfMission(missionID), missionID);
            }
            m_checkListener.confirmMissionSent();
            m_autoSaveManager.requestSave();
        }
        break;
    }
    case CheckEvent::Submission:
        if (m_apSocket.sendToServer(APProtocol::missionCheck(m_checkListener.getPendingSubmissionId())))
        {
            m_checkListener.confirmSubmissionSent();
            m_autoSaveManager.requestSave();
        }
        break;
    case CheckEvent::None:
        break;
    }

    for (CollectibleTracker* collectible : m_checkListener.getCollectibles())
    {
        if (collectible->hasPending()
            && m_apSocket.sendToServer(collectible->buildCheckMessage(collectible->getPendingIndex())))
        {
            collectible->confirmSent();
        }
    }

    if (m_checkListener.hasPendingSubmissionLevel())
    {
        if (m_apSocket.sendToServer(APProtocol::submissionLevelCheck(m_checkListener.getPendingSubmissionLevelSlot())))
        {
            m_checkListener.confirmSubmissionLevelSent();
        }
    }

    if (m_pendingShopChecks.hasPending())
    {
        if (m_apSocket.sendToServer(APProtocol::shopCheck(m_pendingShopChecks.front())))
        {
            m_pendingShopChecks.confirm();
        }
    }
}

void Mod::updateMenuState()
{
    m_apSocket.update();

    std::string rawLine;
    while (m_apSocket.tryGetMessage(rawLine))
    {
        APProtocol::Message message = APProtocol::parse(rawLine);
        if (message.kind == APProtocol::MessageKind::Control)
        {
            applyControlMessage(message.effect, message.text);
        }
        else
        {
            m_deferredLines.push_back(rawLine);
        }
    }

    MenuGate::update(SaveRedirect::isActive());
}

void Mod::parseIncomingMessages()
{
    for (const std::string& line : m_deferredLines)
    {
        handleMessage(line);
    }
    m_deferredLines.clear();

    std::string rawLine;
    while (m_apSocket.tryGetMessage(rawLine))
    {
        handleMessage(rawLine);
    }

    applyPendingItems();
}

void Mod::handleMessage(const std::string& t_rawLine)
{
    APProtocol::Message message = APProtocol::parse(t_rawLine);

    switch (message.kind)
    {
    case APProtocol::MessageKind::Status:
        m_notificationOverlay.show(message.text);
        break;

    case APProtocol::MessageKind::ItemSent:
        m_notificationOverlay.show(message.text, NotificationIcon::ItemSent);
        break;

    case APProtocol::MessageKind::Locate:
        m_checkListener.locateCollectible(message.effect, message.index);
        if (message.index >= 0)
        {
            m_notificationOverlay.show("Locating " + collectibleLabel(message.effect)
                + " #" + std::to_string(message.index + 1));
        }
        break;

    case APProtocol::MessageKind::ShopItem:
        m_ammuNationShop.setSlotContents(message.index, message.text);
        break;

    case APProtocol::MessageKind::ShopSold:
        m_ammuNationShop.setSlotSold(message.index, message.text == "1");
        break;

    case APProtocol::MessageKind::ShopFlags:
        m_ammuNationShop.setSlotFlags(message.index, parseIntOr(message.text, 0));
        break;

    case APProtocol::MessageKind::Give:
        m_receivedItemLog.recordDelivered(message.index, message.effect, message.text);
        break;

    case APProtocol::MessageKind::Control:
        applyControlMessage(message.effect, message.text);
        break;

    case APProtocol::MessageKind::Unknown:
        break;
    }
}

void Mod::applyControlMessage(const std::string& t_name, const std::string& t_value)
{
    if (t_name == "death_link")
    {
        m_deathLinkHandler.setEnabled(t_value == "1");
    }
    else if (t_name == "deathlink_kill")
    {
        m_deathLinkHandler.killPlayer();
    }
    else if (t_name == "collectibles")
    {
        m_checkListener.setIncludedCollectibles(t_value);
    }
    else if (t_name == "gated")
    {
        m_checkListener.setGatedContent(t_value);
    }
    else if (t_name == "world")
    {
        SaveRedirect::setWorld(t_value);
        StartingSaves::seedIfNeeded();
    }
    else if (t_name == "start")
    {
        StartingSaves::setStartingPoint(t_value);
        StartingSaves::seedIfNeeded();
    }
    else if (t_name == "street_races" && t_value == "1")
    {
        StreetRaceUnlock::blockVanillaUnlock();
    }
    else if (t_name == "wang_cars" && t_value == "1")
    {
        WangCarsUnlock::blockVanillaUnlock();
    }
}

void Mod::applyPendingItems()
{
    if (!m_firstInGameTickHandled) return;

    if (m_newGameRegrantPending)
    {
        if (!PlayerControl::isInControl())
        {
            m_newGameRegrantClockStarted = false;
            return;
        }
        unsigned int now = CTimer::m_snTimeInMilliseconds;
        if (!m_newGameRegrantClockStarted || now < m_newGameRegrantControlStartMs)
        {
            m_newGameRegrantClockStarted = true;
            m_newGameRegrantControlStartMs = now;
            return;
        }
        if (now - m_newGameRegrantControlStartMs < NEW_GAME_REGRANT_DELAY_MS) return;
        m_newGameRegrantPending = false;
    }

    std::vector<ReceivedItem> pending = m_receivedItemLog.takePendingItems();
    if (pending.empty()) return;

    int restoredCount = 0;
    for (const ReceivedItem& item : pending)
    {
        if (applyItemEffect(item.effect, item.value, item.isNew) && !item.isNew)
        {
            restoredCount++;
        }
    }

    if (restoredCount > 0)
    {
        m_notificationOverlay.show("Archipelago: Restored " + std::to_string(restoredCount) + " items");
    }
}

bool Mod::applyItemEffect(const std::string& t_effectName, const std::string& t_value, bool t_isNew)
{
    const ItemEffectSpec* spec = findItemEffect(t_effectName);
    if (!spec) return false;

    if (spec->effect == ItemEffect::Trap && !t_isNew) return true;

    switch (spec->effect)
    {
    case ItemEffect::Money:              m_checkGiver.giveMoney(parseIntOr(t_value, 0)); break;
    case ItemEffect::Weapon:             m_checkGiver.giveWeapon(t_value); break;
    case ItemEffect::ProgressiveMission: m_branchProgress.receiveItem(t_value); break;
    case ItemEffect::ProgressiveMap:     m_checkGiver.giveProgressiveMap(); break;
    case ItemEffect::SubmissionCheck:    m_checkListener.submissionCheckWasReceived(spec->submissionId); break;
    case ItemEffect::CollectibleUnlock:  m_checkListener.collectibleUnlockWasReceived(spec->trapName); break;
    case ItemEffect::SubmissionUnlock:   m_checkListener.submissionUnlockWasReceived(spec->submissionId); break;
    case ItemEffect::MaxSkill:           m_checkGiver.giveMaxSkill(spec->submissionId); break;
    case ItemEffect::WeaponMastery:      m_checkListener.submissionCheckWasReceived(shootingRangeSubmissionForWeapon(t_value)); m_checkGiver.giveWeaponMastery(t_value); break;
    case ItemEffect::ArmorRefill:        m_checkGiver.giveArmorRefill(); break;
    case ItemEffect::CarRepair:          m_checkGiver.giveCarRepair(); break;
    case ItemEffect::StreetRaces:        m_streetRacesUnlocked = true; break;
    case ItemEffect::WangCars:           m_wangCarsUnlocked = true; break;
    case ItemEffect::Trap:               m_trapHandler.giveTrap(spec->trapName); break;
    }

    if (t_isNew)
    {
        bool isProgressive = spec->effect == ItemEffect::ProgressiveMission;
        std::string message = formatItemMessage(*spec, isProgressive ? branchDisplayName(t_value) : t_value);
        if (!message.empty())
        {
            int radarSprite = isProgressive ? branchRadarSprite(t_value) : -1;
            m_notificationOverlay.show(message, spec->icon, radarSprite);
        }
    }
    return true;
}

void Mod::drawOverlay()
{
    m_notificationOverlay.draw();
    m_blipManager.drawNumbers();
    drawMissionCounts();
    m_trapHandler.drawTimers();
    if (ModSettings::fastTravelEnabled())
    {
        FastTravel::placeMarkers();
        FastTravel::draw();
    }
}

const char* Mod::branchAtBlip(const CVector& t_pos) const
{
    size_t count = missionStartPos.size();
    if (count > MISSION_START_POS_BRANCH_COUNT) count = MISSION_START_POS_BRANCH_COUNT;

    for (size_t i = 0; i < count; ++i)
    {
        float dx = t_pos.x - missionStartPos[i].x;
        float dy = t_pos.y - missionStartPos[i].y;
        if (dx * dx + dy * dy < MISSION_BLIP_TOLERANCE_SQ) return activeBranchAtMarker(i, m_branchProgress);
    }
    return nullptr;
}

void Mod::drawMissionCounts()
{
    drawMissionCountsImpl(false);
}

void Mod::drawMissionCountsOnMap()
{
    if (!FrontEndMenuManager.m_bMenuActive || FrontEndMenuManager.m_nCurrentMenuPage != MENUPAGE_MAP) return;
    drawMissionCountsImpl(true);
}

void Mod::drawMissionCountsImpl(bool t_menuMap)
{
    float scaleX = t_menuMap ? ScreenScale::of(0.5f) : ModSettings::missionCounterScale();
    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetScale(scaleX, scaleX * 2.0f);
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_CENTER);
    CFont::SetDropShadowPosition(1);
    CFont::SetBackground(false, false);

    for (unsigned int t = 0; t < MAX_RADAR_TRACES; ++t)
    {
        const tRadarTrace& trace = CRadar::ms_RadarTrace[t];
        if (!trace.m_bInUse) continue;

        const char* branch = branchAtBlip(trace.m_vecPos);
        if (!branch) continue;

        CVector2D screenPos;
        if (t_menuMap)
        {
            if (!MenuMap::worldToScreen(trace.m_vecPos, screenPos)) continue;

            float offset = ScreenScale::of(7.0f);
            screenPos.x += offset;
            screenPos.y += offset;
        }
        else
        {
            CVector2D radarSpace;
            CVector2D worldPos(trace.m_vecPos.x, trace.m_vecPos.y);
            CRadar::TransformRealWorldPointToRadarSpace(radarSpace, worldPos);
            if (radarSpace.x * radarSpace.x + radarSpace.y * radarSpace.y > 0.85f * 0.85f) continue;
            CRadar::TransformRadarPointToScreenSpace(screenPos, radarSpace);

            float offset = ScreenScale::of(7.0f);
            screenPos.x += offset;
            screenPos.y += offset;
        }

        int pending = m_branchProgress.pending(branch);
        CFont::SetColor(pending > 0 ? CRGBA(120, 255, 120, 255) : CRGBA(255, 70, 70, 255));
        CFont::PrintString(screenPos.x, screenPos.y, std::to_string(pending).c_str());
    }
}

void Mod::drawCollectiblesOnMap()
{
    m_blipManager.drawMapOverlay();
}

void Mod::drawMenuOverlay()
{
    drawVersionLabel();

    if (m_tagBlipToggleKey.justPressed())
    {
        m_blipManager.toggleBlips();
    }

    bool connected = m_apSocket.isConnected();
    float bottom = static_cast<float>(RsGlobal.maximumHeight);

    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetScale(ScreenScale::of(0.7f), ScreenScale::of(1.4f));
    CFont::SetColor(connected ? CRGBA(80, 220, 80, 255) : CRGBA(220, 80, 80, 255));
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetDropShadowPosition(1);
    CFont::SetBackground(false, false);
    CFont::SetWrapx(static_cast<float>(RsGlobal.maximumWidth));

    CFont::PrintString(ScreenScale::of(20.0f), bottom - ScreenScale::of(100.0f),
        connected ? "Archipelago: Connected" : "Archipelago: Disconnected");

    if (MenuGate::shouldExplainBlock())
    {
        CFont::SetColor(CRGBA(220, 180, 60, 255));
        CFont::PrintString(ScreenScale::of(20.0f), bottom - ScreenScale::of(145.0f),
            "Connect the Archipelago client before starting or loading a game");
    }
    else if (!StartingSaves::missingSaveName().empty())
    {
        std::string warning = "Missing scripts\\Archipelago\\" + StartingSaves::missingSaveName()
            + " - reinstall the starting saves";

        CFont::SetColor(CRGBA(220, 180, 60, 255));
        CFont::PrintString(ScreenScale::of(20.0f), bottom - ScreenScale::of(145.0f), warning.c_str());
    }

    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetScale(ScreenScale::of(0.55f), ScreenScale::of(1.1f));
    CFont::SetColor(CRGBA(255, 255, 255, 255));
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetDropShadowPosition(1);
    CFont::SetBackground(false, false);

    CFont::PrintString(ScreenScale::of(20.0f), bottom - ScreenScale::of(55.0f),
        m_blipManager.areBlipsEnabled() ? "F8 - Collectible blips on map: ON" : "F8 - Collectible blips on map: OFF");
}

void Mod::drawVersionLabel()
{
    CFont::SetFontStyle(FONT_SUBTITLES);
    CFont::SetScale(ScreenScale::of(0.65f), ScreenScale::of(1.3f));
    CFont::SetColor(CRGBA(255, 255, 255, 90));
    CFont::SetProportional(true);
    CFont::SetOrientation(ALIGN_LEFT);
    CFont::SetDropShadowPosition(0);
    CFont::SetBackground(false, false);
    CFont::SetWrapx(static_cast<float>(RsGlobal.maximumWidth));

    CFont::PrintString(ScreenScale::of(20.0f), ScreenScale::of(20.0f),
        ("Archipelago v" + std::string(MOD_VERSION)).c_str());
}

void Mod::spawnCollectiblePickups()
{
	spawnPickupOnce(SPRAYCAN_PICKUP_POS, MODEL_SPRAYCAN, SPRAYCAN_PICKUP_AMMO);
	spawnPickupOnce(CAMERA_PICKUP_POS, MODEL_CAMERA, CAMERA_PICKUP_AMMO);
	spawnPickupOnce(JETPACK_PICKUP_POS, MODEL_JETPACK, 0);
}

void Mod::spawnPickupOnce(const CVector& t_position, int t_modelId, unsigned int t_ammo)
{
	for (int i = 0; i < 620; ++i)
	{
		const CPickup& pickup = CPickups::aPickUps[i];
		if (pickup.m_nPickupType == PICKUP_NONE) continue;
		if (pickup.m_nModelIndex != t_modelId) continue;

		CVector pos = const_cast<CPickup&>(pickup).GetPosn();
		if (std::fabs(pos.x - t_position.x) < 2.0f && std::fabs(pos.y - t_position.y) < 2.0f)
		{
			return;
		}
	}

	CPickups::GenerateNewOne(t_position, t_modelId, PICKUP_ON_STREET, t_ammo, 0, false, nullptr);
}

void Mod::persistAndRestoreState(bool t_worldWiped, bool t_loadHooked)
{
	m_saveDataManager.poll();

	bool firstInGameTick = FindPlayerPed() && !m_firstInGameTickHandled;
	if (firstInGameTick)
	{
		m_firstInGameTickHandled = true;

		if (CStats::LastMissionPassedName[0] == '\0')
		{
			m_newGameRegrantPending = true;
			m_newGameRegrantClockStarted = false;
		}
	}

	bool restoreNeeded = false;
	if (t_loadHooked)
	{
		restoreNeeded = m_saveDataManager.restoreFromCurrentLoadName();
	}

	if (firstInGameTick || t_worldWiped || t_loadHooked)
	{
		spawnCollectiblePickups();

		LegacyBlockerCleanup::removeStaleBlockers();
	}

	if (restoreNeeded)
	{
		m_notificationOverlay.show("Archipelago: Restored progress (" + m_saveDataManager.getCurrentSaveKey() + ")");

		for (PersistentState* subsystem : m_persistentSubsystems)
		{
			subsystem->load(m_saveDataManager);
		}
	}

}

void Mod::resetForNewGame()
{
	SaveDataManager freshDefaults;
	for (PersistentState* subsystem : m_persistentSubsystems)
	{
		subsystem->load(freshDefaults);
	}

	m_blipManager.onWorldWiped();

	m_firstInGameTickHandled = false;
	m_newGameRegrantPending = true;
	m_newGameRegrantClockStarted = false;
}
