#include "ReceivedItemLog.h"
#include "SaveDataManager.h"
#include "ParseUtils.h"

namespace
{
	constexpr char LAST_APPLIED_ITEM_INDEX_KEY[] = "last_applied_item_index";
}

void ReceivedItemLog::save(SaveDataManager& t_saveData)
{
	t_saveData.setValue(LAST_APPLIED_ITEM_INDEX_KEY, std::to_string(m_lastAppliedIndex));
}

void ReceivedItemLog::load(const SaveDataManager& t_saveData)
{
	// Deliberately allowed to move backwards. Loading a save from before some items arrived
	// means that save never received them, so they are owed to it again - the buffer still holds
	// them, and takePendingItems will hand them back on the next tick.
	m_lastAppliedIndex = parseIntOr(
		t_saveData.getValue(LAST_APPLIED_ITEM_INDEX_KEY, std::to_string(NOTHING_APPLIED)),
		NOTHING_APPLIED);
}

void ReceivedItemLog::recordDelivered(int t_index, const std::string& t_effect, const std::string& t_value)
{
	if (t_index < 0) return;

	ReceivedItem item;
	item.index = t_index;
	item.effect = t_effect;
	item.value = t_value;
	m_delivered[t_index] = item;
}

std::vector<ReceivedItem> ReceivedItemLog::takePendingItems()
{
	std::vector<ReceivedItem> pending;

	// std::map iterates in key order, so items are always applied in the order the server sent
	// them - which matters for anything order-dependent and costs nothing here.
	for (const auto& entry : m_delivered)
	{
		const ReceivedItem& item = entry.second;
		if (item.index <= m_lastAppliedIndex) continue;

		ReceivedItem toApply = item;
		// Above the session high-water mark means no save in this process has ever had it, so it
		// is a genuine new drop. At or below means this process already applied it once and we
		// are only here because the mark rolled back to an older save.
		toApply.isNew = item.index > m_sessionHighWaterIndex;
		pending.push_back(toApply);

		m_lastAppliedIndex = item.index;
		if (item.index > m_sessionHighWaterIndex)
		{
			m_sessionHighWaterIndex = item.index;
		}
	}

	return pending;
}
