// CoH Upkeep: the tuning, read from the CoHUpkeep block of the pack (Data/INI/10_Upkeep.ini). Back to
// these defaults after every game.

#pragma once

#include "Common/AsciiString.h"
#include "Common/KindOf.h"

#include "Features/Upkeep/Domain/Population.h"
#include "Features/Upkeep/Domain/Upkeep.h"

#include <vector>

class INI;

namespace CoH::Upkeep
{

// A finished building of this kind raises its owner's cap by 'amount'.
struct BuildingBonus
{
	KindOfType kind = KINDOF_INVALID;
	int amount = 0;
};

// The population of one unit, set by name instead of by its build cost.
struct PopulationOverride
{
	AsciiString unit;
	int population = 0;
};

struct Config
{
	CostRules cost;
	CapRules cap;
	UpkeepSteps steps;
	std::vector<BuildingBonus> buildingBonuses;		///< each CapBuilding line adds one
	std::vector<PopulationOverride> overrides;		///< each Population line adds one

	KindOfMaskType unitKinds;		///< objects of any of these kinds take population...
	KindOfMaskType nonUnitKinds;	///< ...unless they are any of these
	KindOfMaskType neverRefusedKinds;	///< take population but are made even over the cap (workers)

	int aiMaxUpkeepPercent = 25;	///< the AI grows its army only while upkeep stays at or below this

	Config();
};

const Config &config();
void parseConfigBlock(INI *ini);
void resetConfig();

} // namespace CoH::Upkeep
