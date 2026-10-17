// CoH Sectors: see BuildRule.h.

#include "BuildRule.h"

namespace CoH::Sectors
{

bool mayBuildIn(int sector, PlayerId player, bool isDefense, const std::vector<PlayerId> &owners,
	const std::vector<std::vector<int>> &neighbors, const BuildRules &rules)
{
	if (!rules.ownedSectorsOnly)
		return true;
	if (sector < 0 || sector >= (int)owners.size())
		return false;
	if (owners[sector] == player)
		return true;
	if (!isDefense || !rules.defensesOnNeutralNeighbors || owners[sector] != NoPlayer || sector >= (int)neighbors.size())
		return false;

	for (int next : neighbors[sector])
	{
		if (next >= 0 && next < (int)owners.size() && owners[next] == player)
			return true;
	}
	return false;
}

} // namespace CoH::Sectors
