#include "BlipTarget.h"
#include <algorithm>
#include <utility>

void rankByDistance(std::vector<BlipTarget>& t_targets, const CVector& t_playerPos)
{
	std::vector<std::pair<float, int>> byDistance;
	byDistance.reserve(t_targets.size());

	for (int i = 0; i < static_cast<int>(t_targets.size()); ++i)
	{
		if (t_targets[i].claimed) continue;
		byDistance.push_back({ t_playerPos.Distance2D(t_targets[i].position), i });
	}

	std::sort(byDistance.begin(), byDistance.end());

	for (int r = 0; r < static_cast<int>(byDistance.size()); ++r)
	{
		BlipTarget& target = t_targets[byDistance[r].second];
		target.rank = r;
		target.distance = byDistance[r].first;
	}
}
