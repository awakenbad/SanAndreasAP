#include "BranchProgress.h"

#include <set>

#include "ParseUtils.h"
#include "SaveDataManager.h"

namespace
{
	constexpr char PROGRESSIVE_BRANCHES_KEY[] = "progressive_branches";
	constexpr char COMPLETED_MISSIONS_KEY[] = "completed_missions";
	constexpr char BRANCH_SEP = ';';
	constexpr char FIELD_SEP = '|';
}

void BranchProgress::receiveItem(const std::string& t_branch)
{
	if (t_branch.empty()) return;
	m_received[t_branch]++;
}

void BranchProgress::completeMission(const std::string& t_branch, int t_missionId)
{
	m_completedMissions.insert(t_missionId);

	if (t_branch.empty()) return;
	m_completed[t_branch]++;
}

bool BranchProgress::isBlocked(const std::string& t_branch) const
{
	if (t_branch.empty()) return false;

	auto received = m_received.find(t_branch);
	auto completed = m_completed.find(t_branch);
	int receivedCount = received == m_received.end() ? 0 : received->second;
	int completedCount = completed == m_completed.end() ? 0 : completed->second;
	return receivedCount <= completedCount;
}

int BranchProgress::pending(const std::string& t_branch) const
{
	if (t_branch.empty()) return 0;

	auto received = m_received.find(t_branch);
	auto completed = m_completed.find(t_branch);
	int receivedCount = received == m_received.end() ? 0 : received->second;
	int completedCount = completed == m_completed.end() ? 0 : completed->second;
	return receivedCount - completedCount;
}

bool BranchProgress::missionCompleted(int t_missionId) const
{
	return m_completedMissions.count(t_missionId) != 0;
}

int BranchProgress::completed(const std::string& t_branch) const
{
	auto completed = m_completed.find(t_branch);
	return completed == m_completed.end() ? 0 : completed->second;
}

void BranchProgress::save(SaveDataManager& t_saveData)
{
	std::set<std::string> branches;
	for (const auto& entry : m_received) branches.insert(entry.first);
	for (const auto& entry : m_completed) branches.insert(entry.first);

	std::string blob;
	for (const std::string& name : branches)
	{
		auto received = m_received.find(name);
		auto completed = m_completed.find(name);
		int receivedCount = received == m_received.end() ? 0 : received->second;
		int completedCount = completed == m_completed.end() ? 0 : completed->second;
		if (receivedCount == 0 && completedCount == 0) continue;

		if (!blob.empty()) blob += BRANCH_SEP;
		blob += name + FIELD_SEP + std::to_string(receivedCount) + FIELD_SEP + std::to_string(completedCount);
	}
	t_saveData.setValue(PROGRESSIVE_BRANCHES_KEY, blob);

	std::string missions;
	for (int missionId : m_completedMissions)
	{
		if (!missions.empty()) missions += BRANCH_SEP;
		missions += std::to_string(missionId);
	}
	t_saveData.setValue(COMPLETED_MISSIONS_KEY, missions);
}

void BranchProgress::load(const SaveDataManager& t_saveData)
{
	m_received.clear();
	m_completed.clear();
	m_completedMissions.clear();

	std::string missions = t_saveData.getValue(COMPLETED_MISSIONS_KEY, "");
	size_t missionStart = 0;
	while (missionStart < missions.size())
	{
		size_t missionEnd = missions.find(BRANCH_SEP, missionStart);
		if (missionEnd == std::string::npos) missionEnd = missions.size();

		int missionId = parseIntOr(missions.substr(missionStart, missionEnd - missionStart), -1);
		if (missionId >= 0) m_completedMissions.insert(missionId);
		missionStart = missionEnd + 1;
	}

	std::string blob = t_saveData.getValue(PROGRESSIVE_BRANCHES_KEY, "");
	size_t start = 0;
	while (start < blob.size())
	{
		size_t end = blob.find(BRANCH_SEP, start);
		if (end == std::string::npos) end = blob.size();
		std::string entry = blob.substr(start, end - start);
		start = end + 1;

		size_t firstSep = entry.find(FIELD_SEP);
		if (firstSep == std::string::npos) continue;
		size_t secondSep = entry.find(FIELD_SEP, firstSep + 1);
		if (secondSep == std::string::npos) continue;

		std::string name = entry.substr(0, firstSep);
		int receivedCount = parseIntOr(entry.substr(firstSep + 1, secondSep - firstSep - 1), 0);
		int completedCount = parseIntOr(entry.substr(secondSep + 1), 0);
		if (receivedCount != 0) m_received[name] = receivedCount;
		if (completedCount != 0) m_completed[name] = completedCount;
	}
}
