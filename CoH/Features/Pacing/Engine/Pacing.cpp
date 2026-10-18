// CoH Pacing: the feature. Reads the CoHPacing block and scales costs, build times and the AI's income
// through the hub, and the AI scripts' money at game start (AIScripts.cpp). See ../README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/ThingTemplate.h"

#include "Kernel/Ini.h"
#include "Kernel/Kernel.h"
#include "Kernel/Objects.h"

#include "Features/Pacing/Domain/Pacing.h"
#include "Features/Pacing/Engine/Internal.h"
#include "Features/Pacing/Pacing.h"

namespace CoH::Pacing
{

namespace
{

struct Config	// the CoHPacing block of Data/INI/10_Pacing.ini; these are the defaults (retail)
{
	Rules rules;
	KindOfMaskType exemptKinds = kinds(KINDOF_DOZER, KINDOF_HARVESTER, KINDOF_DRONE);
	Handicaps handicaps {};
	Real aiScriptMoney = 1.0f;	///< the share of the AI scripts' gifts of money kept
};

Config &config()
{
	static Config theConfig;
	return theConfig;
}

const Feature *&theFeature()
{
	static const Feature *feature = nullptr;
	return feature;
}

Scale &classScale(UnitClass unitClass)
{
	return config().rules.classes[static_cast<std::size_t>(unitClass)];
}

Real nextPercent(INI *ini)
{
	return INI::scanPercentToReal(ini->getNextToken());
}

// <Class>Cost = 150%, <Class>BuildTime = 150%
template <UnitClass unitClass, float Scale::*field>
void parseClassPercent(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	classScale(unitClass).*field = nextPercent(ini);
}

// Object = <object> <cost> <build time>
void parseObject(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	ObjectScale line;
	line.name = ini->getNextToken();
	line.scale.cost = nextPercent(ini);
	line.scale.buildTime = nextPercent(ini);
	config().rules.objects.push_back(line);
}

// AIEasy = <income> <build time>, and AIMedium, AIHard
template <Difficulty difficulty>
void parseHandicap(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	Handicap &handicap = config().handicaps[static_cast<std::size_t>(difficulty)];
	handicap.income = nextPercent(ini);
	handicap.buildTime = nextPercent(ini);
}

void parseConfigBlock(INI *ini)
{
	static const Ini::Table<Config> fields = {
		Ini::custom<Config>("InfantryCost", parseClassPercent<UnitClass::Infantry, &Scale::cost>),
		Ini::custom<Config>("InfantryBuildTime", parseClassPercent<UnitClass::Infantry, &Scale::buildTime>),
		Ini::custom<Config>("VehicleCost", parseClassPercent<UnitClass::Vehicle, &Scale::cost>),
		Ini::custom<Config>("VehicleBuildTime", parseClassPercent<UnitClass::Vehicle, &Scale::buildTime>),
		Ini::custom<Config>("AircraftCost", parseClassPercent<UnitClass::Aircraft, &Scale::cost>),
		Ini::custom<Config>("AircraftBuildTime", parseClassPercent<UnitClass::Aircraft, &Scale::buildTime>),
		Ini::custom<Config>("StructureCost", parseClassPercent<UnitClass::Structure, &Scale::cost>),
		Ini::custom<Config>("StructureBuildTime", parseClassPercent<UnitClass::Structure, &Scale::buildTime>),
		Ini::field("CostStep", &Config::rules, &Rules::costStep),
		Ini::field("ExemptKinds", &Config::exemptKinds),
		Ini::custom<Config>("Object", parseObject),
		Ini::custom<Config>("AIEasy", parseHandicap<Difficulty::Easy>),
		Ini::custom<Config>("AIMedium", parseHandicap<Difficulty::Medium>),
		Ini::custom<Config>("AIHard", parseHandicap<Difficulty::Hard>),
		Ini::percent("AIScriptMoney", &Config::aiScriptMoney),
	};
	fields.read(ini, config());
}

UnitClass classOf(const ThingTemplate &build)
{
	const Kinds facts = {
		.exempt = build.isAnyKindOf(config().exemptKinds) != FALSE,
		.structure = build.isKindOf(KINDOF_STRUCTURE) != FALSE,
		.aircraft = build.isKindOf(KINDOF_AIRCRAFT) != FALSE,
		.vehicle = build.isKindOf(KINDOF_VEHICLE) != FALSE,
		.infantry = build.isKindOf(KINDOF_INFANTRY) != FALSE,
	};
	return classify(facts);
}

Scale scaleOf(const ThingTemplate &build)
{
	return scaleFor(config().rules, build.getName().str(), classOf(build));
}

// The skirmish AI's handicap; none for humans.
Handicap handicapOf(const Player *player)
{
	if (player == nullptr || !const_cast<Player *>(player)->isSkirmishAIPlayer())
		return {};
	switch (player->getPlayerDifficulty())
	{
		case DIFFICULTY_EASY: return config().handicaps[static_cast<std::size_t>(Difficulty::Easy)];
		case DIFFICULTY_NORMAL: return config().handicaps[static_cast<std::size_t>(Difficulty::Medium)];
		case DIFFICULTY_HARD: return config().handicaps[static_cast<std::size_t>(Difficulty::Hard)];
		default: return {};
	}
}

void adjustBuildCost(const Player * /*player*/, const ThingTemplate *build, int &cost)
{
	if (build != nullptr)
		cost = scaled(cost, scaleOf(*build).cost, config().rules.costStep);
}

void adjustBuildTime(const Player *player, const ThingTemplate *build, int &frames)
{
	if (build != nullptr)
		frames = scaled(frames, scaleOf(*build).buildTime * handicapOf(player).buildTime);
}

void adjustIncome(const Player *player, unsigned int &amount)
{
	amount = scaledIncome(amount, handicapOf(player).income);
}

void onGameStartAfterMap(bool /*loadingSaveGame*/)
{
	scaleAIScriptMoney(config().aiScriptMoney);
}

void init(Feature &self)
{
	theFeature() = &self;
	registerIniBlock("CoHPacing", parseConfigBlock);
	Hooks::buildCost().subscribe(self, adjustBuildCost);
	Hooks::buildTime().subscribe(self, adjustBuildTime);
	Hooks::income().subscribe(self, adjustIncome);
	Hooks::gameStartAfterMap().subscribe(self, onGameStartAfterMap);
}

void reset()
{
	config() = Config();
}

} // namespace

bool isActive()
{
	return theFeature() != nullptr && theFeature()->isActive();
}

float costMultiplier(const ThingTemplate *build)
{
	return isActive() && build != nullptr ? scaleOf(*build).cost : 1.0f;
}

float buildTimeMultiplier(const ThingTemplate *build)
{
	return isActive() && build != nullptr ? scaleOf(*build).buildTime : 1.0f;
}

float incomeMultiplier(const Player *player)
{
	return isActive() ? handicapOf(player).income : 1.0f;
}

} // namespace CoH::Pacing

COH_FEATURE(Pacing, CoH::Pacing::init, CoH::Pacing::reset)
