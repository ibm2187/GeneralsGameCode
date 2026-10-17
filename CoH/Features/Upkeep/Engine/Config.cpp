// CoH Upkeep: see Config.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"

#include "Features/Upkeep/Engine/Config.h"

#include <cstddef>

namespace CoH::Upkeep
{

namespace
{

Config &storage()
{
	static Config theConfig;
	return theConfig;
}

// UpkeepStep = <population> <percent>
void parseStep(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	UpkeepStep step;
	step.population = INI::scanInt(ini->getNextToken());
	step.percent = INI::scanInt(ini->getNextToken());
	((Config *)instance)->steps.add(step);
}

// CapBuilding = <KINDOF> <amount>
void parseBuildingBonus(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	const char *kindName = ini->getNextToken();
	const Int kind = KindOfMaskType::getSingleBitFromName(kindName);
	if (kind < 0)
	{
		DEBUG_CRASH(("CoHUpkeep: unknown kind %s in CapBuilding", kindName));
		throw INI_INVALID_DATA;
	}
	BuildingBonus bonus;
	bonus.kind = (KindOfType)kind;
	bonus.amount = INI::scanInt(ini->getNextToken());
	((Config *)instance)->buildingBonuses.push_back(bonus);
}

// Population = <object> <population>
void parseOverride(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	PopulationOverride entry;
	entry.unit = ini->getNextToken();
	entry.population = INI::scanInt(ini->getNextToken());
	((Config *)instance)->overrides.push_back(entry);
}

const FieldParse configFields[] =
{
	{ "BuildCostPerPopulation",	INI::parseInt,	nullptr, offsetof(Config, cost.buildCostPerPopulation) },
	{ "MinUnitPopulation",		INI::parseInt,	nullptr, offsetof(Config, cost.minPopulation) },
	{ "Population",				parseOverride,	nullptr, 0 },
	{ "UnitKinds",				KindOfMaskType::parseFromINI, nullptr, offsetof(Config, unitKinds) },
	{ "NonUnitKinds",			KindOfMaskType::parseFromINI, nullptr, offsetof(Config, nonUnitKinds) },
	{ "NeverRefusedKinds",		KindOfMaskType::parseFromINI, nullptr, offsetof(Config, neverRefusedKinds) },

	{ "CapBase",				INI::parseInt,	nullptr, offsetof(Config, cap.base) },
	{ "CapPerSector",			INI::parseInt,	nullptr, offsetof(Config, cap.perSector) },
	{ "CapMax",					INI::parseInt,	nullptr, offsetof(Config, cap.max) },
	{ "CapBuilding",			parseBuildingBonus, nullptr, 0 },

	{ "UpkeepStep",				parseStep,		nullptr, 0 },
	{ "AIMaxUpkeepPercent",		INI::parseInt,	nullptr, offsetof(Config, aiMaxUpkeepPercent) },
	{ nullptr, nullptr, nullptr, 0 }
};

} // namespace

Config::Config()
{
	unitKinds.set(KINDOF_INFANTRY);
	unitKinds.set(KINDOF_VEHICLE);
	unitKinds.set(KINDOF_AIRCRAFT);
	nonUnitKinds.set(KINDOF_STRUCTURE);
	nonUnitKinds.set(KINDOF_DRONE);
	neverRefusedKinds.set(KINDOF_DOZER);
	neverRefusedKinds.set(KINDOF_HARVESTER);
}

void parseConfigBlock(INI *ini)
{
	// The pack's steps, bonuses and overrides replace the defaults rather than adding to them.
	Config &config = storage();
	config.steps.clear();
	config.buildingBonuses.clear();
	config.overrides.clear();
	ini->initFromINI(&config, configFields);
}

const Config &config()
{
	return storage();
}

void resetConfig()
{
	storage() = Config();
}

} // namespace CoH::Upkeep
