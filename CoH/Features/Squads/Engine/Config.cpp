// CoH Squads: see Config.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"

#include "Features/Squads/Engine/Config.h"

#include <cstddef>

namespace CoH::Squads
{

namespace
{

Config &storage()
{
	static Config theConfig;
	return theConfig;
}

// Squad = <unit> <size> [<escort unit> <count>]...
void parseSquad(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	SquadType type;
	type.unit = ini->getNextToken();
	type.size = INI::scanInt(ini->getNextToken());
	if (type.size < 1)
	{
		DEBUG_CRASH(("CoHSquads: squad %s needs a size of at least 1", type.unit.c_str()));
		throw INI_INVALID_DATA;
	}
	for (const char *token = ini->getNextTokenOrNull(); token != nullptr; token = ini->getNextTokenOrNull())
	{
		Escort escort;
		escort.unit = token;
		escort.count = INI::scanInt(ini->getNextToken());
		type.escorts.push_back(escort);
	}
	((Config *)instance)->types.add(type);
}

const FieldParse configFields[] =
{
	{ "Squad",			parseSquad,		nullptr, 0 },
	{ "CostPercent",	INI::parseInt,	nullptr, offsetof(Config, costPercent) },
	{ "TimePercent",	INI::parseInt,	nullptr, offsetof(Config, timePercent) },
	{ "Spacing",		INI::parseReal,	nullptr, offsetof(Config, formation.spacing) },
	{ "RowWidth",		INI::parseInt,	nullptr, offsetof(Config, formation.rowWidth) },
	{ nullptr, nullptr, nullptr, 0 }
};

} // namespace

void parseConfigBlock(INI *ini)
{
	// The pack's squads replace the defaults (none) rather than adding to them.
	Config &config = storage();
	config.types.clear();
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

} // namespace CoH::Squads
