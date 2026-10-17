// CoH Upkeep: the answers of the public header, Upkeep.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Features/Upkeep/Engine/Internal.h"
#include "Features/Upkeep/Upkeep.h"

namespace CoH::Upkeep
{

bool isActive()
{
	return isUpkeepGame();
}

int population(const Player *player)
{
	return isUpkeepGame() ? standingOf(player).population : 0;
}

int populationCap(const Player *player)
{
	return isUpkeepGame() ? standingOf(player).cap : 0;
}

int upkeepPercent(const Player *player)
{
	return isUpkeepGame() ? standingOf(player).percent : 0;
}

int populationCost(const ThingTemplate *unit)
{
	return isUpkeepGame() ? unitPopulation(unit) : 0;
}

} // namespace CoH::Upkeep
