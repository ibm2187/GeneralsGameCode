// CoH Sectors: see Network.h.

#include "Network.h"

#include <cmath>

namespace CoH::Sectors
{

std::vector<bool> findConnected(const std::vector<std::vector<int>> &neighbors, const std::vector<PlayerId> &owners,
	PlayerId player, const std::vector<int> &roots)
{
	const int count = (int)owners.size();
	std::vector<bool> connected(count, false);
	std::vector<int> open;

	for (int root : roots)
	{
		if (root >= 0 && root < count && owners[root] == player && !connected[root])
		{
			connected[root] = true;
			open.push_back(root);
		}
	}

	while (!open.empty())
	{
		const int sector = open.back();
		open.pop_back();
		if (sector >= (int)neighbors.size())
			continue;

		for (int next : neighbors[sector])
		{
			if (next >= 0 && next < count && !connected[next] && owners[next] == player)
			{
				connected[next] = true;
				open.push_back(next);
			}
		}
	}
	return connected;
}

int sectorValue(SectorKind kind, const IncomeValues &values)
{
	switch (kind)
	{
		case SectorKind::HighValue: return values.highValue;
		case SectorKind::Home: return values.home;
		case SectorKind::Supply: break;
	}
	return values.supply;
}

int incomeFor(const std::vector<SectorKind> &kinds, const std::vector<bool> &connected, const IncomeValues &values)
{
	int income = 0;
	for (size_t i = 0; i < kinds.size() && i < connected.size(); ++i)
	{
		if (connected[i])
			income += sectorValue(kinds[i], values);
	}
	return income;
}

int incomePerMinute(int incomePerPeriod, float periodSeconds)
{
	if (periodSeconds <= 0.0f)
		return 0;
	return (int)std::lround(incomePerPeriod * 60.0f / periodSeconds);
}

} // namespace CoH::Sectors
