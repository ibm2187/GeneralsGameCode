// CoH Veterancy: see Config.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/Object.h"

#include "Kernel/Ini.h"

#include "Features/Veterancy/Engine/Config.h"

#include <cstring>
#include <map>

namespace CoH::Veterancy
{

namespace
{

Config &storage()
{
	static Config theConfig;
	return theConfig;
}

// Unit types asked about this game, so a shot or a step doesn't match names. Only looked up.
std::map<const ThingTemplate *, RankTable> &knownUnits()
{
	static std::map<const ThingTemplate *, RankTable> units;
	return units;
}

bool is(const char *token, const char *name)
{
	return _stricmp(token, name) == 0;
}

Rank parseRank(const char *token)
{
	static constexpr const char *names[RankCount] = { "REGULAR", "VETERAN", "ELITE", "HEROIC" };
	for (std::size_t i = 0; i < RankCount; ++i)
	{
		if (is(token, names[i]))
			return static_cast<Rank>(i);
	}
	DEBUG_CRASH(("CoHVeterancy: unknown rank %s (REGULAR, VETERAN, ELITE or HEROIC)", token));
	throw INI_INVALID_DATA;
}

// The rest of a line: <bonus> <value> pairs, such as "Accuracy 120% Upgrade Upgrade_AmericaBattleDrone".
BonusChange parseChange(INI *ini)
{
	BonusChange change;
	while (const char *key = ini->getNextTokenOrNull())
	{
		const char *value = ini->getNextToken();
		if (is(key, "Accuracy"))
			change.accuracy = INI::scanPercentToReal(value);
		else if (is(key, "DamageTaken"))
			change.damageTaken = INI::scanPercentToReal(value);
		else if (is(key, "SuppressionResistance"))
			change.suppressionResistance = INI::scanPercentToReal(value);
		else if (is(key, "Speed"))
			change.speed = INI::scanPercentToReal(value);
		else if (is(key, "Sight"))
			change.sight = INI::scanPercentToReal(value);
		else if (is(key, "Reload"))
			change.reload = INI::scanPercentToReal(value);
		else if (is(key, "Upgrade"))
			change.upgrade = std::string(value);
		else
		{
			DEBUG_CRASH(("CoHVeterancy: unknown bonus %s", key));
			throw INI_INVALID_DATA;
		}
	}
	return change;
}

// Infantry = <rank> <bonus> <value> ..., and Vehicle, Aircraft
template <UnitClass unitClass>
void parseClass(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	const Rank rank = parseRank(ini->getNextToken());
	parseChange(ini).applyTo(storage().rules.classes[static_cast<std::size_t>(unitClass)][static_cast<std::size_t>(rank)]);
}

// Object = <object> <rank> <bonus> <value> ...
void parseObject(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	ObjectLine line;
	line.name = ini->getNextToken();
	line.rank = parseRank(ini->getNextToken());
	line.change = parseChange(ini);
	storage().rules.objects.push_back(line);
}

UnitClass classOf(const ThingTemplate &unit)
{
	const Kinds facts = {
		.aircraft = unit.isKindOf(KINDOF_AIRCRAFT) != FALSE,
		.vehicle = unit.isKindOf(KINDOF_VEHICLE) != FALSE,
		.infantry = unit.isKindOf(KINDOF_INFANTRY) != FALSE,
	};
	return classify(facts);
}

} // namespace

const Config &config()
{
	return storage();
}

void parseConfigBlock(INI *ini)
{
	static const Ini::Table<Config> fields = {
		Ini::percent("DamageExperience", &Config::damageExperience),
		Ini::field("SquadsShare", &Config::squadsShare),
		Ini::custom<Config>("Infantry", parseClass<UnitClass::Infantry>),
		Ini::custom<Config>("Vehicle", parseClass<UnitClass::Vehicle>),
		Ini::custom<Config>("Aircraft", parseClass<UnitClass::Aircraft>),
		Ini::custom<Config>("Object", parseObject),
	};
	fields.read(ini, storage());
	knownUnits().clear();
}

void resetConfig()
{
	storage() = Config();
	knownUnits().clear();
}

const RankTable &tableOf(const ThingTemplate *unit)
{
	static const RankTable none {};
	if (unit == nullptr)
		return none;
	auto found = knownUnits().find(unit);
	if (found == knownUnits().end())
		found = knownUnits().emplace(unit, tableFor(config().rules, unit->getName().str(), classOf(*unit))).first;
	return found->second;
}

const RankBonus &bonusOf(const Object *unit)
{
	static const RankBonus none {};
	if (unit == nullptr)
		return none;
	return tableOf(unit->getTemplate())[static_cast<std::size_t>(rankOf((int)unit->getVeterancyLevel()))];
}

} // namespace CoH::Veterancy
