// CoH Sectors: see AiTargets.h.

#include "AiTargets.h"

#include <algorithm>
#include <cmath>

namespace CoH::Sectors
{

namespace
{

bool isConnected(const std::vector<bool> &connected, int sector)
{
	return sector >= 0 && sector < (int)connected.size() && connected[sector];
}

bool bordersConnected(const std::vector<int> &around, const std::vector<bool> &connected)
{
	return std::any_of(around.begin(), around.end(), [&](int n) { return isConnected(connected, n); });
}

bool bordersCutOff(PlayerId player, const std::vector<int> &around, const std::vector<AiSector> &sectors,
	const std::vector<bool> &connected)
{
	return std::any_of(around.begin(), around.end(), [&](int n)
	{
		return n >= 0 && n < (int)sectors.size() && sectors[n].owner == player && !isConnected(connected, n);
	});
}

// The score of a sector the player doesn't hold, or of a held one under attack. Not a target: nothing.
bool scoreTarget(PlayerId player, int index, const std::vector<AiSector> &sectors,
	const std::vector<std::vector<int>> &neighbors, const std::vector<bool> &connected, const AiWeights &weights,
	float &score)
{
	const AiSector &sector = sectors[index];
	static const std::vector<int> none;
	const std::vector<int> &around = index < (int)neighbors.size() ? neighbors[index] : none;

	if (sector.owner == player)
	{
		score = weights.defend;
		return sector.threatened;
	}
	if (sector.ownerIsFriend || sector.locked)
		return false;

	score = 0.0f;
	if (bordersConnected(around, connected))
		score += weights.frontier;
	if (bordersCutOff(player, around, sectors, connected))
		score += weights.reconnect;
	if (sector.kind == SectorKind::HighValue)
		score += weights.highValue;
	if (sector.owner != NoPlayer)
		score += weights.enemyOwned;
	return true;
}

} // namespace

std::vector<int> rankTargets(PlayerId player, const std::vector<AiSector> &sectors,
	const std::vector<std::vector<int>> &neighbors, const std::vector<bool> &connected, Vec2 base,
	const AiWeights &weights)
{
	struct Scored
	{
		int index;
		float score;
	};
	std::vector<Scored> targets;

	for (int i = 0; i < (int)sectors.size(); ++i)
	{
		float score = 0.0f;
		if (!scoreTarget(player, i, sectors, neighbors, connected, weights, score))
			continue;

		score += std::sqrt(distanceSquared(sectors[i].position, base)) / 1000.0f * weights.distancePer1000;
		targets.push_back({ i, score });
	}

	std::stable_sort(targets.begin(), targets.end(), [](const Scored &a, const Scored &b) { return a.score > b.score; });

	std::vector<int> order;
	for (const Scored &target : targets)
		order.push_back(target.index);
	return order;
}

} // namespace CoH::Sectors
