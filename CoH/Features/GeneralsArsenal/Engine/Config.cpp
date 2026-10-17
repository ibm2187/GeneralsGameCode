// CoH GeneralsArsenal: see Config.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "Common/SpecialPower.h"

#include "Features/GeneralsArsenal/Domain/Cooldown.h"
#include "Features/GeneralsArsenal/Engine/Config.h"

#include <cstddef>

namespace CoH::GeneralsArsenal
{

namespace
{

Config &storage()
{
	static Config theConfig;
	return theConfig;
}

const char *const AiUseNames[] = { "NONE", "STRIKE", "COVER", "SUPERWEAPON", nullptr };

// Level = <science> <rank>
void parseLevel(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	PowerConfig *power = (PowerConfig *)instance;
	PowerLevel level;
	level.science = ini->getNextToken();
	level.rank = INI::scanInt(ini->getNextToken());
	power->rule.levels.push_back(level);
}

const FieldParse configFields[] =
{
	{ "PointsPerKillExperience",	INI::parseReal,	nullptr, offsetof(Config, earn.perKillExperience) },
	{ "PointsPerDamageExperience",	INI::parseReal,	nullptr, offsetof(Config, earn.perDamageExperience) },
	{ "MaxPoints",					INI::parseReal,	nullptr, offsetof(Config, earn.maxPoints) },
	{ "MaxLossDelay",				INI::parseReal,	nullptr, offsetof(Config, maxLossDelaySeconds) },
	{ "AirfieldKinds",				KindOfMaskType::parseFromINI, nullptr, offsetof(Config, airfieldKinds) },
	{ "AIRemoveFromBuildList",		KindOfMaskType::parseFromINI, nullptr, offsetof(Config, aiBuildListRemovals) },
	{ "AIThinkInterval",			INI::parseReal,	nullptr, offsetof(Config, aiThinkSeconds) },
	{ "AISuperweaponSaving",		INI::parseReal,	nullptr, offsetof(Config, aiPolicy.superweaponSavingShare) },
	{ "AIStrikeMinValue",			INI::parseInt,	nullptr, offsetof(Config, aiStrikeMinValue) },
	{ "AIStrikeRadius",				INI::parseReal,	nullptr, offsetof(Config, aiStrikeRadius) },
	{ "AICoverRadius",				INI::parseReal,	nullptr, offsetof(Config, aiCoverRadius) },
	{ "AIFirstUse",					INI::parseReal,	nullptr, offsetof(Config, aiFirstUseSeconds) },
	{ "LedgerObject",				INI::parseAsciiString, nullptr, offsetof(Config, ledgerObject) },
	{ nullptr, nullptr, nullptr, 0 }
};

const FieldParse powerFields[] =
{
	{ "Cost",				INI::parseInt,		nullptr, offsetof(PowerConfig, rule.cost) },
	{ "MinRank",			INI::parseInt,		nullptr, offsetof(PowerConfig, rule.minRank) },
	{ "NeedsAirfield",		INI::parseBool,		nullptr, offsetof(PowerConfig, rule.needsAirfield) },
	{ "LossPenalty",		INI::parseReal,		nullptr, offsetof(PowerConfig, rule.lossPenaltySeconds) },
	{ "AIUse",				INI::parseIndexList, AiUseNames, offsetof(PowerConfig, aiUse) },
	{ "AITargetKinds",		KindOfMaskType::parseFromINI, nullptr, offsetof(PowerConfig, aiTargetKinds) },
	{ "Description",		INI::parseAsciiString, nullptr, offsetof(PowerConfig, description) },
	{ "Level",				parseLevel,			nullptr, 0 },
	{ nullptr, nullptr, nullptr, 0 }
};

} // namespace

Config::Config()
{
	airfieldKinds.set(KINDOF_FS_AIRFIELD);
	aiBuildListRemovals.set(KINDOF_FS_SUPERWEAPON);
	ledgerObject = "CoHCommandLedger";
}

void parseConfigBlock(INI *ini)
{
	ini->initFromINI(&storage(), configFields);
}

// CoHPower <SpecialPowerTemplate>: a later block for the same power replaces the earlier one.
void parsePowerBlock(INI *ini)
{
	const std::string name = ini->getNextToken();
	PowerConfig power;
	power.aiTargetKinds.set(KINDOF_VEHICLE);
	power.aiTargetKinds.set(KINDOF_INFANTRY);
	ini->initFromINI(&power, powerFields);
	storage().powers[name] = power;
}

const Config &config()
{
	return storage();
}

void resetConfig()
{
	storage() = Config();
}

const PowerConfig *powerConfig(const SpecialPowerTemplate *power)
{
	if (power == nullptr || storage().powers.empty())
		return nullptr;
	const auto it = storage().powers.find(power->getName().str());
	return it != storage().powers.end() ? &it->second : nullptr;
}

AiUse aiUseOf(const PowerConfig &power)
{
	switch (power.aiUse)
	{
		case 1: return AiUse::Strike;
		case 2: return AiUse::Cover;
		case 3: return AiUse::Superweapon;
		default: return AiUse::None;
	}
}

UnsignedInt secondsToLogicFrames(Real seconds)
{
	return secondsToFrames(seconds, LOGICFRAMES_PER_SECOND);
}

} // namespace CoH::GeneralsArsenal
