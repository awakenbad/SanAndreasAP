#pragma once
#include "PersistentState.h"

#include <map>
#include <set>
#include <string>

class BranchProgress : public PersistentState
{
public:
	void save(SaveDataManager& t_saveData) override;
	void load(const SaveDataManager& t_saveData) override;

	void receiveItem(const std::string& t_branch);
	void completeMission(const std::string& t_branch, int t_missionId);
	bool isBlocked(const std::string& t_branch) const;
	int pending(const std::string& t_branch) const;
	bool missionCompleted(int t_missionId) const;
	int completed(const std::string& t_branch) const;

private:
	std::map<std::string, int> m_received;
	std::map<std::string, int> m_completed;
	std::set<int> m_completedMissions;
};
