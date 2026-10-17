// CoH Sectors: where a player may build. Players build only in sectors they own; base defenses may
// also go in neutral sectors next to an owned one. Both parts are switched in INI.

#pragma once

#include "Types.h"

#include <vector>

namespace CoH::Sectors
{

struct BuildRules
{
	bool ownedSectorsOnly = true;			///< off: build anywhere, as in the original game
	bool defensesOnNeutralNeighbors = true;	///< defenses may go in a neutral sector bordering an owned one
};

bool mayBuildIn(int sector, PlayerId player, bool isDefense, const std::vector<PlayerId> &owners,
	const std::vector<std::vector<int>> &neighbors, const BuildRules &rules);

} // namespace CoH::Sectors
