// CoH Upkeep: population. Each unit takes population by its build cost; the cap starts low and rises
// with buildings and with sectors held. A unit may be made only while it fits under the cap.

#pragma once

namespace CoH::Upkeep
{

struct CostRules
{
	int buildCostPerPopulation = 250;	///< a unit takes one population per this much of its build cost
	int minPopulation = 1;				///< a unit that costs money takes at least this much
};

// The population a unit takes. A unit that costs nothing (called-in planes, spawned helpers) takes
// none. 'override' (-1 for none) is a value the pack sets for that unit.
int populationCost(int buildCost, int override, const CostRules &rules);

struct CapRules
{
	int base = 10;			///< every player has this much
	int perSector = 3;		///< each sector the player holds adds this much
	int max = 150;			///< the cap never goes higher
};

// The cap for a player whose buildings add 'buildingBonus' and who holds 'heldSectors'.
int populationCap(int buildingBonus, int heldSectors, const CapRules &rules);

// A unit taking 'cost' may be made: it fits under the cap. A unit taking nothing always fits.
bool fitsUnderCap(int population, int cost, int cap);

} // namespace CoH::Upkeep
