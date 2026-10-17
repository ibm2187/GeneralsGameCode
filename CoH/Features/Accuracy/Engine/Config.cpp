// CoH Accuracy: see Config.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "GameLogic/Weapon.h"

#include "Features/Accuracy/Engine/Config.h"

#include <cstddef>
#include <unordered_map>

namespace CoH::Accuracy
{

namespace
{

Config &storage()
{
	static Config theConfig;
	return theConfig;
}

// Weapons asked about this game, so a shot doesn't look its weapon up by name.
std::unordered_map<const WeaponTemplate *, float> &knownWeapons()
{
	static std::unordered_map<const WeaponTemplate *, float> theWeapons;
	return theWeapons;
}

int scanPercent(INI *ini)
{
	return REAL_TO_INT_FLOOR(INI::scanPercentToReal(ini->getNextToken(ini->getSepsPercent())) * 100.0f + 0.5f);
}

// Class = <DamageType> <percent>
void parseClass(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	const char *typeName = ini->getNextToken();
	const Int type = DamageTypeFlags::getSingleBitFromName(typeName);
	if (type < 0)
	{
		DEBUG_CRASH(("CoHAccuracy: unknown damage type %s in Class", typeName));
		throw INI_INVALID_DATA;
	}
	((Config *)instance)->classPercent[type] = scanPercent(ini);
}

// Weapon = <weapon> <percent>
void parseWeapon(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	WeaponAccuracy entry;
	entry.weapon = ini->getNextToken();
	entry.percent = scanPercent(ini);
	((Config *)instance)->weapons.push_back(entry);
}

// TargetSize = <Infantry|Vehicle|Aircraft|Building> <percent>
void parseTargetSize(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	static const char *const sizeNames[] = { "Infantry", "Vehicle", "Aircraft", "Building" };
	static_assert(ARRAY_SIZE(sizeNames) == (int)TargetSize::Count, "a name for every TargetSize");

	const char *sizeName = ini->getNextToken();
	for (Int i = 0; i < (Int)TargetSize::Count; ++i)
	{
		if (stricmp(sizeName, sizeNames[i]) == 0)
		{
			((Config *)instance)->rules.sizeMultiplier[i] = INI::scanPercentToReal(ini->getNextToken(ini->getSepsPercent()));
			return;
		}
	}
	DEBUG_CRASH(("CoHAccuracy: unknown target size %s", sizeName));
	throw INI_INVALID_DATA;
}

const FieldParse configFields[] =
{
	{ "Class",				parseClass,					nullptr, 0 },
	{ "Weapon",				parseWeapon,				nullptr, 0 },
	{ "MaxDamageRadius",	INI::parseReal,				nullptr, offsetof(Config, rules.maxDamageRadius) },

	{ "MinChance",			INI::parsePercentToReal,	nullptr, offsetof(Config, rules.minChance) },
	{ "MaxChance",			INI::parsePercentToReal,	nullptr, offsetof(Config, rules.maxChance) },

	{ "NearRange",			INI::parsePercentToReal,	nullptr, offsetof(Config, rules.nearRange) },
	{ "FarRange",			INI::parsePercentToReal,	nullptr, offsetof(Config, rules.farRange) },
	{ "Near",				INI::parsePercentToReal,	nullptr, offsetof(Config, rules.nearMultiplier) },
	{ "Mid",				INI::parsePercentToReal,	nullptr, offsetof(Config, rules.midMultiplier) },
	{ "Far",				INI::parsePercentToReal,	nullptr, offsetof(Config, rules.farMultiplier) },

	{ "ShooterMoving",		INI::parsePercentToReal,	nullptr, offsetof(Config, rules.shooterMovingMultiplier) },
	{ "TargetMoving",		INI::parsePercentToReal,	nullptr, offsetof(Config, rules.targetMovingMultiplier) },
	{ "MovingSpeed",		INI::parseReal,				nullptr, offsetof(Config, movingSpeed) },
	{ "TargetSize",			parseTargetSize,			nullptr, 0 },
	{ "VeterancyPerRank",	INI::parsePercentToReal,	nullptr, offsetof(Config, rules.veterancyPerRank) },

	{ "MissDistanceMin",	INI::parseReal,				nullptr, offsetof(Config, rules.missDistanceMin) },
	{ "MissDistanceMax",	INI::parseReal,				nullptr, offsetof(Config, rules.missDistanceMax) },
	{ "MissFX",				INI::parseAsciiString,		nullptr, offsetof(Config, missFX) },
	{ nullptr, nullptr, nullptr, 0 }
};

// The weapon's own line, or -1.
int ownPercent(const Config &config, const WeaponTemplate *weapon)
{
	for (const WeaponAccuracy &entry : config.weapons)
	{
		if (entry.weapon.compareNoCase(weapon->getName()) == 0)
			return entry.percent;
	}
	return -1;
}

float lookUpBaseAccuracy(const WeaponTemplate *weapon)
{
	const Config &config = storage();
	const WeaponBonus noBonus;

	WeaponFacts facts;
	const DamageType type = weapon->getDamageType();
	facts.classPercent = (type >= 0 && type < DAMAGE_NUM_TYPES) ? config.classPercent[type] : -1;
	facts.weaponPercent = ownPercent(config, weapon);
	facts.damageRadius = max(weapon->getPrimaryDamageRadius(noBonus), weapon->getSecondaryDamageRadius(noBonus));
	facts.scatterRadius = weapon->getScatterRadius();
	facts.contact = weapon->isContactWeapon();
	facts.laser = weapon->getLaserName().isNotEmpty();
	facts.damageAtSelf = weapon->getDamageDealtAtSelfPosition();
	return baseAccuracy(config.rules, facts);
}

} // namespace

Config::Config()
{
	for (int &percent : classPercent)
		percent = -1;
}

void parseConfigBlock(INI *ini)
{
	// The block starts from the defaults: its Class and Weapon lines are the whole list.
	Config &config = storage();
	config = Config();
	knownWeapons().clear();
	ini->initFromINI(&config, configFields);
}

const Config &config()
{
	return storage();
}

void resetConfig()
{
	storage() = Config();
	knownWeapons().clear();
}

float weaponBaseAccuracy(const WeaponTemplate *weapon)
{
	if (weapon == nullptr)
		return -1.0f;

	std::unordered_map<const WeaponTemplate *, float> &known = knownWeapons();
	const auto found = known.find(weapon);
	if (found != known.end())
		return found->second;

	const float base = lookUpBaseAccuracy(weapon);
	known.emplace(weapon, base);
	return base;
}

} // namespace CoH::Accuracy
