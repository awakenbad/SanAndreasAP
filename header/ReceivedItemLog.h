#pragma once
#include <map>
#include <string>
#include <vector>
#include "PersistentState.h"

// One item the client has delivered, identified by its position in the server's items_received
// list. The index is stable for the life of a seed: the server always replays the same list in
// the same order, which is what makes it usable as a durable "already applied" mark.
class ReceivedItem
{
public:
	int index = -1;
	std::string effect;
	std::string value;
	// False when this item is being re-delivered to a save that had already progressed past it
	// (see ReceivedItemLog::takePendingItems). One-shot effects like traps must not fire again.
	bool isNew = true;
};

// Decides which delivered items this save has already been given.
//
// The problem it solves: the AP server replays the entire items_received list on every connect,
// so a client restart re-delivers everything. Nothing on the client can prevent that - its own
// counter resets with the process. The mark has to live in the GTA save, because the save is the
// thing that can roll back: loading an older save SHOULD re-grant the items it never saw, and
// that only works if the mark travels with it.
class ReceivedItemLog : public PersistentState
{
public:
	void save(SaveDataManager& t_saveData) override;
	void load(const SaveDataManager& t_saveData) override;

	// Buffers one delivered item. Safe to call with indices already seen or already applied -
	// filtering happens in takePendingItems, so delivery order and repetition don't matter.
	void recordDelivered(int t_index, const std::string& t_effect, const std::string& t_value);

	// Hands back everything above the save's mark, in index order, and advances the mark past
	// them. Returns empty when there is nothing to apply, which is the normal case on a
	// reconnect - the whole list arrives and every item is already accounted for.
	std::vector<ReceivedItem> takePendingItems();

private:
	// Highest index applied by THIS save. Persisted; moves backwards when an older save loads.
	int m_lastAppliedIndex = NOTHING_APPLIED;
	// Highest index applied by THIS PROCESS, across whatever saves it has loaded. Not persisted:
	// its only job is telling "genuinely new" apart from "re-delivered after a rollback", and
	// that distinction is meaningless once the process ends.
	int m_sessionHighWaterIndex = NOTHING_APPLIED;

	// Keyed by index so repeated deliveries collapse and iteration is automatically in order.
	std::map<int, ReceivedItem> m_delivered;

	// -1 rather than 0, since 0 is a real item index. A save with no mark has had nothing
	// applied - which for a save written before this key existed means its items are re-granted
	// once, announced as restored.
	static constexpr int NOTHING_APPLIED = -1;
};
