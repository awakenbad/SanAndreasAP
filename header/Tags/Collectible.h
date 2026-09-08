#pragma once
#include <CRGBA.h>
#include <array>
#include <span>
#include <string>
#include <vector>

#include "APProtocol.h"
#include "BlipTarget.h"
#include "PendingChecks.h"
#include "SaveDataManager.h"

class CollectibleTracker
{
public:
	virtual ~CollectibleTracker() = default;

	virtual bool update() = 0;
	virtual void resyncBaseline() = 0;
	virtual void save(SaveDataManager& t_saveData) const = 0;
	virtual void load(const SaveDataManager& t_saveData) = 0;

	virtual bool hasPending() const = 0;
	virtual int getPendingIndex() const = 0;
	virtual void confirmSent() = 0;
	virtual std::string buildCheckMessage(int t_index) const = 0;

	virtual void appendBlipTargets(std::vector<BlipTarget>& t_out) const = 0;

	virtual const char* checkType() const = 0;

	virtual void unlock() {}
	virtual void setGated(bool t_gated) {}
	virtual bool isUnlocked() const { return true; }
	virtual const char* consumeLockedNotice() { return nullptr; }

	virtual void setLocated(int t_index) = 0;
	virtual void setIncluded(const std::vector<int>& t_indices) = 0;
};

template <int N>
class Collectible : public CollectibleTracker
{
public:

	bool update() override
	{
		float currentCount = readCount();

		if (!m_countInitialized)
		{
			m_lastCount = currentCount;
			m_countInitialized = true;
			return hasPending();
		}

		int delta = static_cast<int>(currentCount) - static_cast<int>(m_lastCount);

		if (delta < 0)
		{
			m_lastCount = currentCount;
			return hasPending();
		}

		for (int i = 0; i < delta; ++i)
		{
			int index = identifyCollected();
			if (index < 0) break;

			m_lastCount += 1.0f;
			if (m_claimed[index]) continue;

			if (!isIncluded(index)) continue;

			m_claimed[index] = true;
			m_pending.push(index);
		}

		return hasPending();
	}

	void resyncBaseline() override { m_lastCount = readCount(); }

	bool hasPending() const override { return m_pending.hasPending(); }
	int getPendingIndex() const override { return m_pending.hasPending() ? m_pending.front() : -1; }
	void confirmSent() override { m_pending.confirm(); }

	std::string buildCheckMessage(int t_index) const override { return APProtocol::collectibleCheck(m_checkType, t_index); }

	void appendBlipTargets(std::vector<BlipTarget>& t_out) const override
	{
		if (m_positions.empty()) return;
		for (int i = 0; i < N; ++i)
		{
			bool claimed = m_claimed[i] || !isIncluded(i) || !isUnlocked();
			t_out.push_back({ m_positions[i], getHeightIndicatorColor(), m_sprite, i + 1, claimed, i == locatedIndex(), INT_MAX });
		}
	}

	const std::array<bool, N>& getClaimed() const { return m_claimed; }

	void unlock() override { m_unlocked = true; }
	void setGated(bool t_gated) override { m_gated = t_gated; }
	bool isUnlocked() const override { return !m_gated || m_unlocked; }

	const char* checkType() const override { return m_checkType; }

	void setLocated(int t_index) override
	{
		m_located = (t_index >= 0 && t_index < N) ? t_index : -1;
	}

	void setIncluded(const std::vector<int>& t_indices) override
	{
		m_included.fill(false);
		for (int index : t_indices)
		{
			if (index >= 0 && index < N) m_included[index] = true;
		}
		m_hasInclusionMask = true;
	}

	bool isIncluded(int t_index) const { return !m_hasInclusionMask || m_included[t_index]; }

	void save(SaveDataManager& t_saveData) const override
	{
		std::string bits(N, '0');
		for (int i = 0; i < N; ++i)
		{
			if (m_claimed[i]) bits[i] = '1';
		}
		t_saveData.setValue(m_saveKey, bits);
		t_saveData.setValue(std::string(m_saveKey) + "_unlocked", m_unlocked ? "1" : "0");
	}

	void load(const SaveDataManager& t_saveData) override
	{
		std::string bits = t_saveData.getValue(m_saveKey, std::string(N, '0'));
		for (int i = 0; i < N; ++i)
		{
			m_claimed[i] = i < static_cast<int>(bits.size()) && bits[i] == '1';
		}
		m_unlocked = t_saveData.getValue(std::string(m_saveKey) + "_unlocked", "0") == "1";
	}

protected:
	Collectible(std::span<const CVector> t_positions, int t_sprite,
		const char* t_saveKey, const char* t_checkType)
		: m_positions(t_positions), m_sprite(t_sprite), m_saveKey(t_saveKey), m_checkType(t_checkType) {
	}

	virtual float readCount() const = 0;
	virtual int identifyCollected() const = 0;
	int locatedIndex() const { return m_located; }

	bool isClaimed(int t_index) const { return m_claimed[t_index]; }

	virtual CRGBA getHeightIndicatorColor() const { return CRGBA(240, 170, 20, 255); }

private:
	std::span<const CVector> m_positions;
	int m_sprite;
	const char* m_saveKey;
	const char* m_checkType;
	std::array<bool, N> m_claimed{};
	PendingChecks<int> m_pending;
	std::array<bool, N> m_included{};
	bool m_hasInclusionMask = false;
	int m_located = -1;
	float m_lastCount = 0.0f;
	bool m_countInitialized = false;
	bool m_gated = false;
	bool m_unlocked = false;
};
