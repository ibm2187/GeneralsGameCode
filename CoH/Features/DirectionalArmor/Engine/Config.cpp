// CoH DirectionalArmor: see Config.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/INI.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"

#include "Kernel/Ini.h"

#include "Features/DirectionalArmor/Domain/Names.h"
#include "Features/DirectionalArmor/Engine/Config.h"

#include <ranges>
#include <unordered_map>

namespace CoH::DirectionalArmor
{

Config::Config()
{
	std::ranges::fill(infantryDamage, 1.0f);
	infantryDamageFrom.set(KINDOF_VEHICLE);
	infantryDamageNotFrom.set(KINDOF_AIRCRAFT);
	exempt.set(KINDOF_AIRCRAFT);
	arcs = arcsFromDegrees(frontArc, rearArc);
}

namespace
{

Config &storage()
{
	static Config theConfig;
	return theConfig;
}

// Looked up once per template this game; looking up only, never walked, so the order is no matter.
std::unordered_map<const ThingTemplate *, std::optional<ArmorValues>> &knownUnits()
{
	static std::unordered_map<const ThingTemplate *, std::optional<ArmorValues>> theUnits;
	return theUnits;
}

std::unordered_map<const WeaponTemplate *, std::optional<Penetration>> &knownWeapons()
{
	static std::unordered_map<const WeaponTemplate *, std::optional<Penetration>> theWeapons;
	return theWeapons;
}

std::unordered_map<const WeaponTemplate *, std::optional<BlastShell>> &knownBlasts()
{
	static std::unordered_map<const WeaponTemplate *, std::optional<BlastShell>> theBlasts;
	return theBlasts;
}

void forgetLookUps()
{
	knownUnits().clear();
	knownWeapons().clear();
	knownBlasts().clear();
}

Config &configOf(void *instance)
{
	return *static_cast<Config *>(instance);
}

DamageType damageTypeNamed(const char *name, const char *line)
{
	const Int type = DamageTypeFlags::getSingleBitFromName(name);
	if (type < 0)
	{
		DEBUG_CRASH(("CoHDirectionalArmor: unknown damage type %s in %s", name, line));
		throw INI_INVALID_DATA;
	}
	return (DamageType)type;
}

Penetration readPenetration(INI *ini)
{
	Penetration penetration;
	penetration.atNear = INI::scanReal(ini->getNextToken());
	penetration.atFar = INI::scanReal(ini->getNextToken());
	return penetration;
}

ArmorValues readArmor(INI *ini)
{
	ArmorValues armor;
	armor.front = INI::scanReal(ini->getNextToken());
	armor.side = INI::scanReal(ini->getNextToken());
	armor.rear = INI::scanReal(ini->getNextToken());
	return armor;
}

// Penetration = <DamageType> <near> <far>
void parsePenetration(INI *ini, void *instance, void *, const void *)
{
	const DamageType type = damageTypeNamed(ini->getNextToken(), "Penetration");
	configOf(instance).byDamageType[type] = readPenetration(ini);
}

// Weapon = <weapon> <near> <far>
void parseWeapon(INI *ini, void *instance, void *, const void *)
{
	NamedLine<Penetration> line;
	line.name = ini->getNextToken();
	line.values = readPenetration(ini);
	configOf(instance).weapons.push_back(line);
}

// Kind = <KindOf> <front> <side> <rear>
void parseKind(INI *ini, void *instance, void *, const void *)
{
	const char *name = ini->getNextToken();
	const Int kind = KindOfMaskType::getSingleBitFromName(name);
	if (kind < 0)
	{
		DEBUG_CRASH(("CoHDirectionalArmor: unknown KindOf %s in Kind", name));
		throw INI_INVALID_DATA;
	}
	configOf(instance).kinds.push_back({ (KindOfType)kind, readArmor(ini) });
}

// Unit = <object> <front> <side> <rear>
void parseUnit(INI *ini, void *instance, void *, const void *)
{
	NamedLine<ArmorValues> line;
	line.name = ini->getNextToken();
	line.values = readArmor(ini);
	configOf(instance).units.push_back(line);
}

// InfantryDamage = <DamageType> <percent>
void parseInfantryDamage(INI *ini, void *instance, void *, const void *)
{
	const DamageType type = damageTypeNamed(ini->getNextToken(), "InfantryDamage");
	configOf(instance).infantryDamage[type] = INI::scanPercentToReal(ini->getNextToken());
}

// Blast = <weapon> <damage> <radius>
void parseBlast(INI *ini, void *instance, void *, const void *)
{
	NamedLine<BlastShell> line;
	line.name = ini->getNextToken();
	line.values.damage = INI::scanReal(ini->getNextToken());
	line.values.radius = INI::scanReal(ini->getNextToken());
	configOf(instance).blasts.push_back(line);
}

// InfantryBlast = <percent near> <percent at the edge>
void parseInfantryBlast(INI *ini, void *instance, void *, const void *)
{
	BlastFalloff &falloff = configOf(instance).infantryBlast;
	falloff.atInner = INI::scanPercentToReal(ini->getNextToken());
	falloff.atOuter = INI::scanPercentToReal(ini->getNextToken());
}

template <class Values>
const Values *bestNamed(const std::vector<NamedLine<Values>> &lines, const AsciiString &name)
{
	const Values *best = nullptr;
	int bestMatch = 0;
	for (const NamedLine<Values> &line : lines)
	{
		const int match = nameMatch(line.name.str(), name.str());
		if (match > bestMatch)
		{
			best = &line.values;
			bestMatch = match;
		}
	}
	return best;
}

std::optional<ArmorValues> lookUpArmor(const Object &object)
{
	const Config &c = storage();
	if (const ArmorValues *own = bestNamed(c.units, object.getTemplate()->getName()))
		return *own;
	if (object.isAnyKindOf(c.exempt))
		return std::nullopt;
	for (const KindLine &line : c.kinds)
	{
		if (object.isKindOf(line.kind))
			return line.armor;
	}
	return std::nullopt;
}

std::optional<Penetration> lookUpPenetration(const WeaponTemplate &weapon)
{
	const Config &c = storage();
	if (const Penetration *own = bestNamed(c.weapons, weapon.getName()))
		return *own;
	const DamageType type = weapon.getDamageType();
	if (type >= 0 && type < DAMAGE_NUM_TYPES)
		return c.byDamageType[type];
	return std::nullopt;
}

} // namespace

void parseConfigBlock(INI *ini)
{
	static const Ini::Table<Config> fields = {
		Ini::percent("DeflectedDamage", &Config::deflectedDamage),
		Ini::percent("FrontDamage", &Config::sideDamage, &SideDamage::front),
		Ini::percent("SideDamage", &Config::sideDamage, &SideDamage::side),
		Ini::percent("RearDamage", &Config::sideDamage, &SideDamage::rear),
		Ini::field("FrontArc", &Config::frontArc),
		Ini::field("RearArc", &Config::rearArc),
		Ini::field("AreaRadius", &Config::areaRadius),
		Ini::field("DeflectFX", &Config::deflectFX),
		Ini::field("DeflectTextPenetration", &Config::deflectTextPenetration),
		Ini::custom<Config>("Penetration", parsePenetration),
		Ini::custom<Config>("Weapon", parseWeapon),
		Ini::custom<Config>("Kind", parseKind),
		Ini::custom<Config>("Unit", parseUnit),
		Ini::field("Exempt", &Config::exempt),
		Ini::custom<Config>("InfantryDamage", parseInfantryDamage),
		Ini::field("InfantryDamageFrom", &Config::infantryDamageFrom),
		Ini::field("InfantryDamageNotFrom", &Config::infantryDamageNotFrom),
		Ini::custom<Config>("Blast", parseBlast),
		Ini::custom<Config>("InfantryBlast", parseInfantryBlast),
		Ini::field("Flanking", &Config::flanking),
		Ini::percent("FlankBelowChance", &Config::flank, &FlankRules::belowChance),
		Ini::percent("FlankMinGain", &Config::flank, &FlankRules::minGain),
		Ini::percent("FlankRangeShare", &Config::flank, &FlankRules::rangeShare),
		Ini::percent("FlankRearShare", &Config::flank, &FlankRules::rearShare),
		Ini::field("FlankMaxTravel", &Config::flank, &FlankRules::maxTravel),
		Ini::seconds("FlankCheckEvery", &Config::flankCheckFrames),
		Ini::seconds("FlankTimeout", &Config::flankTimeoutFrames),
		Ini::seconds("FlankRest", &Config::flankRestFrames),
	};

	// The block starts from the defaults: its Penetration, Weapon, Kind, Unit and InfantryDamage
	// lines are the whole list.
	Config &c = storage();
	c = Config();
	forgetLookUps();
	fields.read(ini, c);
	c.arcs = arcsFromDegrees(c.frontArc, c.rearArc);
}

const Config &config()
{
	return storage();
}

void resetConfig()
{
	storage() = Config();
	forgetLookUps();
}

const ArmorValues *armorOf(const Object &object)
{
	auto &known = knownUnits();
	auto found = known.find(object.getTemplate());
	if (found == known.end())
		found = known.emplace(object.getTemplate(), lookUpArmor(object)).first;
	return found->second ? &*found->second : nullptr;
}

const Penetration *penetrationOf(const WeaponTemplate &weapon)
{
	auto &known = knownWeapons();
	auto found = known.find(&weapon);
	if (found == known.end())
		found = known.emplace(&weapon, lookUpPenetration(weapon)).first;
	return found->second ? &*found->second : nullptr;
}

const BlastShell *blastOf(const WeaponTemplate &weapon)
{
	auto &known = knownBlasts();
	auto found = known.find(&weapon);
	if (found == known.end())
	{
		const BlastShell *own = bestNamed(storage().blasts, weapon.getName());
		found = known.emplace(&weapon, own != nullptr ? std::optional<BlastShell>(*own) : std::nullopt).first;
	}
	return found->second ? &*found->second : nullptr;
}

Real infantryDamageFactor(DamageType type)
{
	return (type >= 0 && type < DAMAGE_NUM_TYPES) ? storage().infantryDamage[type] : 1.0f;
}

} // namespace CoH::DirectionalArmor
