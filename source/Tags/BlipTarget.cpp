#include "BlipTarget.h"

#include <algorithm>
#include <cmath>
#include <utility>

void rankByDistance(std::vector<BlipTarget>& t_targets, const CVector& t_playerPos)
{
	std::vector<std::pair<float, int>> byDistance;
	byDistance.reserve(t_targets.size());

	for (int i = 0; i < static_cast<int>(t_targets.size()); ++i)
	{
		if (t_targets[i].claimed) continue;
		float dx = t_playerPos.x - t_targets[i].position.x;
		float dy = t_playerPos.y - t_targets[i].position.y;
		byDistance.push_back({ dx * dx + dy * dy, i });
	}

	std::sort(byDistance.begin(), byDistance.end());

	for (int r = 0; r < static_cast<int>(byDistance.size()); ++r)
	{
		BlipTarget& target = t_targets[byDistance[r].second];
		target.rank = r;
		target.distance = std::sqrt(byDistance[r].first);
	}
}
