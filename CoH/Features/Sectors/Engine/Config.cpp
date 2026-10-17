// CoH Sectors: see Config.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"

#include "Features/Sectors/Engine/Config.h"

#include <cstddef>

namespace CoH::Sectors
{

namespace
{

Config &storage()
{
	static Config theConfig;
	return theConfig;
}

void parseScriptRename(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	Config *config = (Config *)instance;
	ScriptRename rename;
	rename.from = ini->getNextAsciiString();
	rename.to = ini->getNextAsciiString();
	config->aiScriptRenames.push_back(rename);
}

const FieldParse configFields[] =
{
	{ "MergeDistance",				INI::parseReal,	nullptr, offsetof(Config, points.mergeDistance) },
	{ "HomeMergeDistance",			INI::parseReal,	nullptr, offsetof(Config, points.homeMergeDistance) },
	{ "CellSize",					INI::parseReal,	nullptr, offsetof(Config, map.cellSize) },
	{ "HomeRadius",					INI::parseReal,	nullptr, offsetof(Config, map.homeRadius) },

	{ "CaptureRadius",				INI::parseReal,	nullptr, offsetof(Config, captureRadius) },
	{ "CaptureInterval",			INI::parseReal,	nullptr, offsetof(Config, captureIntervalSeconds) },
	{ "CaptureSeconds",				INI::parseReal,	nullptr, offsetof(Config, capture.captureSeconds) },
	{ "NeutralizeSeconds",			INI::parseReal,	nullptr, offsetof(Config, capture.neutralizeSeconds) },
	{ "RecoverSeconds",				INI::parseReal,	nullptr, offsetof(Config, capture.recoverSeconds) },
	{ "MaxCapturingUnits",			INI::parseInt,	nullptr, offsetof(Config, capture.maxUnits) },
	{ "ExtraUnitBonus",				INI::parseReal,	nullptr, offsetof(Config, capture.extraUnitBonus) },
	{ "CaptureKinds",				KindOfMaskType::parseFromINI, nullptr, offsetof(Config, captureKinds) },
	{ "NonCaptureKinds",			KindOfMaskType::parseFromINI, nullptr, offsetof(Config, nonCaptureKinds) },
	{ "HighValueKinds",				KindOfMaskType::parseFromINI, nullptr, offsetof(Config, highValueKinds) },

	{ "IncomeSeconds",				INI::parseReal,	nullptr, offsetof(Config, incomeSeconds) },
	{ "SupplyIncome",				INI::parseInt,	nullptr, offsetof(Config, income.supply) },
	{ "HighValueIncome",			INI::parseInt,	nullptr, offsetof(Config, income.highValue) },
	{ "HomeIncome",					INI::parseInt,	nullptr, offsetof(Config, income.home) },

	{ "BuildInOwnedSectorsOnly",	INI::parseBool,	nullptr, offsetof(Config, build.ownedSectorsOnly) },
	{ "DefensesOnNeutralNeighbors",	INI::parseBool,	nullptr, offsetof(Config, build.defensesOnNeutralNeighbors) },
	{ "DefenseKinds",				KindOfMaskType::parseFromINI, nullptr, offsetof(Config, defenseKinds) },

	{ "AIThinkInterval",			INI::parseReal,	nullptr, offsetof(Config, aiThinkSeconds) },
	{ "AIGroupSize",				INI::parseInt,	nullptr, offsetof(Config, aiGroupSize) },
	{ "AIMaxTargets",				INI::parseInt,	nullptr, offsetof(Config, aiMaxTargets) },
	{ "AIDefendWeight",				INI::parseReal,	nullptr, offsetof(Config, ai.defend) },
	{ "AIReconnectWeight",			INI::parseReal,	nullptr, offsetof(Config, ai.reconnect) },
	{ "AIFrontierWeight",			INI::parseReal,	nullptr, offsetof(Config, ai.frontier) },
	{ "AIHighValueWeight",			INI::parseReal,	nullptr, offsetof(Config, ai.highValue) },
	{ "AIEnemyOwnedWeight",			INI::parseReal,	nullptr, offsetof(Config, ai.enemyOwned) },
	{ "AIDistanceWeight",			INI::parseReal,	nullptr, offsetof(Config, ai.distancePer1000) },
	{ "AIRemoveFromBuildList",		KindOfMaskType::parseFromINI, nullptr, offsetof(Config, aiBuildListRemovals) },
	{ "AIScriptRename",				parseScriptRename, nullptr, 0 },

	{ "BorderSmoothing",			INI::parseInt,	nullptr, offsetof(Config, borderLines.smoothing) },
	{ "BorderBandWidth",			INI::parseReal,	nullptr, offsetof(Config, borderBandWidth) },

	{ "PointObject",				INI::parseAsciiString, nullptr, offsetof(Config, pointObject) },
	{ nullptr, nullptr, nullptr, 0 }
};

} // namespace

Config::Config()
{
	captureKinds.set(KINDOF_INFANTRY);
	captureKinds.set(KINDOF_VEHICLE);
	nonCaptureKinds.set(KINDOF_AIRCRAFT);
	nonCaptureKinds.set(KINDOF_DOZER);
	nonCaptureKinds.set(KINDOF_HARVESTER);
	nonCaptureKinds.set(KINDOF_DRONE);
	highValueKinds.set(KINDOF_TECH_BUILDING);
	defenseKinds.set(KINDOF_FS_BASE_DEFENSE);
	aiBuildListRemovals.set(KINDOF_FS_SUPPLY_CENTER);
	aiBuildListRemovals.set(KINDOF_FS_SUPPLY_DROPZONE);
	aiScriptRenames = { { "SupplyCenter", "Barracks" }, { "SupplyStash", "Barracks" }, { "SupplyDropZone", "Barracks" } };
	pointObject = "CoHSectorPoint";
}

void parseConfigBlock(INI *ini)
{
	// The pack's AIScriptRename lines replace the defaults rather than adding to them.
	Config &config = storage();
	const std::vector<ScriptRename> defaults = config.aiScriptRenames;
	config.aiScriptRenames.clear();
	ini->initFromINI(&config, configFields);
	if (config.aiScriptRenames.empty())
		config.aiScriptRenames = defaults;
}

const Config &config()
{
	return storage();
}

void resetConfig()
{
	storage() = Config();
}

UnsignedInt secondsToFrames(Real seconds)
{
	const Int frames = REAL_TO_INT_CEIL(seconds * LOGICFRAMES_PER_SECOND);
	return frames > 0 ? (UnsignedInt)frames : 1;
}

} // namespace CoH::Sectors
