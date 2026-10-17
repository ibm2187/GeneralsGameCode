// CoH Upkeep: a player's population, cap and upkeep share, counted from the player's objects.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/Object.h"
#include "GameLogic/Module/ProductionUpdate.h"

#include "Features/Sectors/Sectors.h"
#include "Features/Squads/Squads.h"
#include "Features/Upkeep/Engine/Config.h"
#include "Features/Upkeep/Engine/Internal.h"

namespace CoH::Upkeep
{

namespace
{

int overriddenPopulation(const ThingTemplate *unit)
{
	for (const PopulationOverride &entry : config().overrides)
	{
		if (entry.unit == unit->getName())
			return entry.population;
	}
	return -1;
}

int queuedPopulation(const Object *factory)
{
	const ProductionUpdateInterface *queue = ProductionUpdate::getProductionUpdateInterfaceFromObject(const_cast<Object *>(factory));
	if (queue == nullptr)
		return 0;

	int population = 0;
	for (const ProductionEntry *entry = queue->firstProduction(); entry != nullptr; entry = queue->nextProduction(entry))
	{
		if (entry->getProductionType() != PRODUCTION_UNIT)
			continue;
		// An order for a squad makes one squad, however many units the factory makes per order.
		const int count = Squads::isSquadUnit(entry->getProductionObject()) || entry->getProductionQuantity() <= 0
			? 1 : entry->getProductionQuantityRemaining();
		population += unitPopulation(entry->getProductionObject()) * count;
	}
	return population;
}

int buildingBonusOf(const Object *object)
{
	if (!object->isKindOf(KINDOF_STRUCTURE) || object->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION) ||
		object->testStatus(OBJECT_STATUS_SOLD))
		return 0;

	int bonus = 0;
	for (const BuildingBonus &entry : config().buildingBonuses)
	{
		if (object->isKindOf(entry.kind))
			bonus += entry.amount;
	}
	return bonus;
}

void countObject(Object *object, void *userData)
{
	if (object->isEffectivelyDead())
		return;

	Standing *standing = (Standing *)userData;
	// A squad counts once, by its leader.
	if (Squads::countsAsUnit(object))
		standing->population += unitPopulation(object->getTemplate());
	standing->population += queuedPopulation(object);
	standing->buildingBonus += buildingBonusOf(object);
}

} // namespace

int unitPopulation(const ThingTemplate *unit)
{
	if (unit == nullptr)
		return 0;
	const Config &rules = config();
	if (!unit->isAnyKindOf(rules.unitKinds) || unit->isAnyKindOf(rules.nonUnitKinds))
		return 0;
	return populationCost(Squads::buildCostOf(unit), overriddenPopulation(unit), rules.cost);
}

bool isNeverRefused(const ThingTemplate *unit)
{
	return unit != nullptr && unit->isAnyKindOf(config().neverRefusedKinds);
}

Standing standingOf(const Player *player)
{
	Standing standing;
	if (player == nullptr)
		return standing;

	player->iterateObjects(countObject, &standing);
	standing.cap = populationCap(standing.buildingBonus, Sectors::heldSectors(player), config().cap);
	standing.percent = config().steps.percentAt(standing.population);
	return standing;
}

} // namespace CoH::Upkeep
