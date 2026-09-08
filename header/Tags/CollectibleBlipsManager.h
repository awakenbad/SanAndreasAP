#pragma once
#include <vector>
#include "BlipTarget.h"
#include "PersistentState.h"

#include <CRadar.h>
#include <CSprite2d.h>

class SaveDataManager;

class CollectibleBlipsManager : public PersistentState
{
public:
	void save(SaveDataManager& t_saveData) override;
	void load(const SaveDataManager& t_saveData) override;

	bool render(std::vector<BlipTarget> t_targets);
	void drawRadarNumbers() const;
	void drawMapOverlay();
	void onWorldWiped();
	void toggleBlips();
	bool areBlipsEnabled() const;

	void clearAllBlips();

private:
	int targetIndexAt(const CVector& t_pos) const;
	int ownedTargetIndexAt(const CVector& t_pos, int t_sprite) const;
	bool ownsBlip(int t_handle) const;
	int findExistingBlipAt(const CVector& t_pos, int t_sprite) const;

	eRadarTraceHeight getRadarTraceHeight(BlipTarget target, CVector playerPos) const;
	void drawBlipHeightOverlay(BlipTarget target, CVector2D spritePos, CVector playerPos, bool onBlip) const;

	void reconcileWithPool();
	void clearStaleSentinels();

	std::vector<BlipTarget> m_targets;
	std::vector<int> m_handles;

	int m_reconcileTicks = RECONCILE_TICKS;
	static constexpr int RECONCILE_TICKS = 10;
	static constexpr int MAX_BLIPS = 30;
	static constexpr float RANGE_HYSTERESIS = 1.15f;
	bool m_blipsEnabled = true;
	int m_sentinelHandle = -1;
};
