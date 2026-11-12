#pragma once
#include <map>
#include <string>

class ItemHistoryEntry
{
public:
	int index = -1;
	int flags = 0;
	std::string item;
	std::string sender;
	std::string game;
	std::string location;
};

class ItemHistory
{
public:
	void record(int t_index, int t_flags, const std::string& t_fields);
	const std::map<int, ItemHistoryEntry>& entries() const;

private:
	std::map<int, ItemHistoryEntry> m_entries;
};
