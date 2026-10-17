// CoH Sectors: building only in owned sectors (Domain/BuildRule), asked by the engine's build
// location check for the player's placement cursor and for the AI's base building alike.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/ThingTemplate.h"

#include "Kernel/Log.h"

#include "Features/Sectors/Domain/BuildRule.h"
#include "Features/Sectors/Engine/Config.h"
#include "Features/Sectors/Engine/Internal.h"
#include "Features/Sectors/Engine/World.h"

namespace CoH::Sectors
{

namespace
{

// How often each player was refused a spot, for the log.
std::vector<int> &refusals()
{
	static std::vector<int> counts(MAX_PLAYER_COUNT, 0);
	return counts;
}

} // namespace

int refusedBuildSpots(int player)
{
	return player >= 0 && player < MAX_PLAYER_COUNT ? refusals()[player] : 0;
}

void forgetRefusedBuildSpots()
{
	refusals().assign(MAX_PLAYER_COUNT, 0);
}

bool isBuildingAllowedAt(const ThingTemplate *build, const Coord3D *position, const Player *player)
{
	const World &w = world();
	if (!w.hasSectors() || build == nullptr || position == nullptr || player == nullptr)
		return true;
	if (!build->isKindOf(KINDOF_STRUCTURE) || !player->isPlayableSide())
		return true;

	const bool isDefense = build->isAnyKindOf(config().defenseKinds);
	return mayBuildIn(w.sectorAt(*position), player->getPlayerIndex(), isDefense, w.ownersAtRefresh(),
		w.map().neighbors(), config().build);
}

bool allowsBuildingAt(const ThingTemplate *build, const Coord3D *position, const Player *player)
{
	const World &w = world();
	const bool allowed = isBuildingAllowedAt(build, position, player);
	if (!allowed && ++refusals()[player->getPlayerIndex()] <= 10 && CoH::isLogging())
	{
		const int sector = w.sectorAt(*position);
		CoH::log("Sectors: refused %s for player %d at (%.0f, %.0f): sector %d owned by %d", build->getName().str(),
			player->getPlayerIndex(), position->x, position->y, sector, sector >= 0 ? w.ownersAtRefresh()[sector] : -2);
	}
	return allowed;
}

} // namespace CoH::Sectors
