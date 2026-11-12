#include "ItemHistory.h"
#include "APProtocol.h"
#include <vector>

namespace
{
	std::string fieldOrEmpty(const std::vector<std::string>& t_fields, size_t t_position)
	{
		return t_position < t_fields.size() ? t_fields[t_position] : std::string();
	}
}

void ItemHistory::record(int t_index, int t_flags, const std::string& t_fields)
{
	if (t_index < 0) return;

	std::vector<std::string> fields = APProtocol::splitList(t_fields, '\t');
	ItemHistoryEntry entry;
	entry.index = t_index;
	entry.flags = t_flags;
	entry.item = fieldOrEmpty(fields, 0);
	entry.sender = fieldOrEmpty(fields, 1);
	entry.game = fieldOrEmpty(fields, 2);
	entry.location = fieldOrEmpty(fields, 3);
	m_entries[t_index] = entry;
}

const std::map<int, ItemHistoryEntry>& ItemHistory::entries() const
{
	return m_entries;
}
