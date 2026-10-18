// CoH Accuracy: see Config.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "GameLogic/Weapon.h"

#include "Features/Accuracy/Engine/Config.h"

#include <algorithm>
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
std::unordered_map<const WeaponTemplate *, RangeProfile> &knownWeapons()
{
	static std::unordered_map<const WeaponTemplate *, RangeProfile> theWeapons;
	return theWeapons;
}

Real scanPercent(const char *token)
{
	return INI::scanPercentToReal(token);
}

// <near%> [<mid%> <far%>]: one value is the same at every range.
RangeProfile parseProfile(INI *ini)
{
	RangeProfile profile;
	profile.nearAccuracy = scanPercent(ini->getNextToken(ini->getSepsPercent()));
	const char *midToken = ini->getNextTokenOrNull(ini->getSepsPercent());
	if (midToken == nullptr)
	{
		profile.midAccuracy = profile.farAccuracy = profile.nearAccuracy;
		return profile;
	}
	profile.midAccuracy = scanPercent(midToken);
	profile.farAccuracy = scanPercent(ini->getNextToken(ini->getSepsPercent()));
	return profile;
}

// Class = <DamageType> <near%> [<mid%> <far%>]
void parseClass(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	const char *typeName = ini->getNextToken();
	const Int type = DamageTypeFlags::getSingleBitFromName(typeName);
	if (type < 0)
	{
		DEBUG_CRASH(("CoHAccuracy: unknown damage type %s in Class", typeName));
		throw INI_INVALID_DATA;
	}
	((Config *)instance)->classProfiles[type] = parseProfile(ini);
}

// Weapon = <weapon> <near%> [<mid%> <far%>]
void parseWeapon(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	WeaponAccuracy entry;
	entry.weapon = ini->getNextToken();
	entry.profile = parseProfile(ini);
	((Config *)instance)->weapons.push_back(entry);
}

// NeverMisses = <weapon> [<weapon> ...]
void parseNeverMisses(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	for (const char *name = ini->getNextTokenOrNull(); name != nullptr; name = ini->getNextTokenOrNull())
		((Config *)instance)->neverMisses.push_back(AsciiString(name));
}

// SizePoint = <radius> <percent>
void parseSizePoint(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	SizePoint point;
	point.radius = INI::scanReal(ini->getNextToken());
	point.multiplier = scanPercent(ini->getNextToken(ini->getSepsPercent()));

	std::vector<SizePoint> &curve = ((Config *)instance)->rules.sizeCurve;
	const auto above = std::find_if(curve.begin(), curve.end(),
		[&point](const SizePoint &other) { return other.radius >= point.radius; });
	if (above != curve.end() && above->radius == point.radius)
		*above = point;
	else
		curve.insert(above, point);
}

const FieldParse configFields[] =
{
	{ "Class",				parseClass,					nullptr, 0 },
	{ "Weapon",				parseWeapon,				nullptr, 0 },
	{ "NeverMisses",		parseNeverMisses,			nullptr, 0 },
	{ "MaxDamageRadius",	INI::parseReal,				nullptr, offsetof(Config, rules.maxDamageRadius) },

	{ "MinChance",			INI::parsePercentToReal,	nullptr, offsetof(Config, rules.minChance) },
	{ "MaxChance",			INI::parsePercentToReal,	nullptr, offsetof(Config, rules.maxChance) },
	{ "MaxChanceBuilding",	INI::parsePercentToReal,	nullptr, offsetof(Config, rules.maxChanceBuilding) },

	{ "SizePoint",			parseSizePoint,				nullptr, 0 },
	{ "BuildingSizeMin",	INI::parsePercentToReal,	nullptr, offsetof(Config, rules.buildingSizeMin) },

	{ "ShooterMoving",		INI::parsePercentToReal,	nullptr, offsetof(Config, rules.shooterMovingMultiplier) },
	{ "TargetMoving",		INI::parsePercentToReal,	nullptr, offsetof(Config, rules.targetMovingMultiplier) },
	{ "MovingSpeed",		INI::parseReal,				nullptr, offsetof(Config, movingSpeed) },
	{ "VeterancyPerRank",	INI::parsePercentToReal,	nullptr, offsetof(Config, rules.veterancyPerRank) },

	{ "MissDistanceMin",	INI::parseReal,				nullptr, offsetof(Config, rules.missDistanceMin) },
	{ "MissDistanceMax",	INI::parseReal,				nullptr, offsetof(Config, rules.missDistanceMax) },
	{ "MissFX",				INI::parseAsciiString,		nullptr, offsetof(Config, missFX) },
	{ nullptr, nullptr, nullptr, 0 }
};

bool isListed(const std::vector<AsciiString> &names, const AsciiString &name)
{
	for (const AsciiString &listed : names)
	{
		if (listed.compareNoCase(name) == 0)
			return true;
	}
	return false;
}

RangeProfile ownProfile(const Config &config, const WeaponTemplate *weapon)
{
	for (const WeaponAccuracy &entry : config.weapons)
	{
		if (entry.weapon.compareNoCase(weapon->getName()) == 0)
			return entry.profile;
	}
	return {};
}

RangeProfile lookUpProfile(const WeaponTemplate *weapon)
{
	const Config &config = storage();
	const WeaponBonus noBonus;

	WeaponFacts facts;
	const DamageType type = weapon->getDamageType();
	if (type >= 0 && type < DAMAGE_NUM_TYPES)
		facts.classProfile = config.classProfiles[type];
	facts.ownProfile = ownProfile(config, weapon);
	facts.neverMisses = isListed(config.neverMisses, weapon->getName());
	facts.damageRadius = max(weapon->getPrimaryDamageRadius(noBonus), weapon->getSecondaryDamageRadius(noBonus));
	facts.scatterRadius = weapon->getScatterRadius();
	facts.contact = weapon->isContactWeapon();
	facts.laser = weapon->getLaserName().isNotEmpty();
	facts.damageAtSelf = weapon->getDamageDealtAtSelfPosition();
	return rollingProfile(config.rules, facts);
}

} // namespace

void parseConfigBlock(INI *ini)
{
	// The block starts from the defaults: its Class, Weapon, NeverMisses and SizePoint lines are the
	// whole list.
	Config &config = storage();
	config = Config();
	config.rules.sizeCurve.clear();
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

const RangeProfile &weaponProfile(const WeaponTemplate *weapon)
{
	static const RangeProfile none;
	if (weapon == nullptr)
		return none;

	std::unordered_map<const WeaponTemplate *, RangeProfile> &known = knownWeapons();
	auto found = known.find(weapon);
	if (found == known.end())
		found = known.emplace(weapon, lookUpProfile(weapon)).first;
	return found->second;
}

} // namespace CoH::Accuracy
