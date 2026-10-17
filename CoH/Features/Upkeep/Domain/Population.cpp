// CoH Upkeep: see Population.h.

#include "Features/Upkeep/Domain/Population.h"

#include <algorithm>

namespace CoH::Upkeep
{

int populationCost(int buildCost, int override, const CostRules &rules)
{
	if (override >= 0)
		return override;
	if (buildCost <= 0 || rules.buildCostPerPopulation <= 0)
		return 0;

	const int rounded = (buildCost + rules.buildCostPerPopulation / 2) / rules.buildCostPerPopulation;
	return std::max(rounded, rules.minPopulation);
}

int populationCap(int buildingBonus, int heldSectors, const CapRules &rules)
{
	const int cap = rules.base + std::max(buildingBonus, 0) + std::max(heldSectors, 0) * rules.perSector;
	return std::min(cap, rules.max);
}

bool fitsUnderCap(int population, int cost, int cap)
{
	return cost <= 0 || population + cost <= cap;
}

} // namespace CoH::Upkeep
