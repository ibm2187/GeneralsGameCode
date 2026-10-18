// CoH FireRate: see Config.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameCommon.h"
#include "Common/INI.h"
#include "GameLogic/Weapon.h"

#include "Kernel/Log.h"

#include "Features/FireRate/Engine/Config.h"

#include <cstddef>
#include <unordered_map>

namespace CoH::FireRate
{

namespace
{

Config &storage()
{
	static Config theConfig;
	return theConfig;
}

// Weapons asked about this game, so a shot doesn't look its weapon up by name.
std::unordered_map<const WeaponTemplate *, Scaling> &knownWeapons()
{
	static std::unordered_map<const WeaponTemplate *, Scaling> theWeapons;
	return theWeapons;
}

// Class = <DamageType> <seconds a shot>
void parseClass(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	const char *typeName = ini->getNextToken();
	const Int type = DamageTypeFlags::getSingleBitFromName(typeName);
	if (type < 0)
	{
		DEBUG_CRASH(("CoHFireRate: unknown damage type %s in Class", typeName));
		throw INI_INVALID_DATA;
	}
	((Config *)instance)->classSeconds[type] = INI::scanReal(ini->getNextToken());
}

// Weapon = <weapon> <seconds a shot>
void parseWeapon(INI *ini, void *instance, void * /*store*/, const void * /*userData*/)
{
	OwnLimit entry;
	entry.weapon = ini->getNextToken();
	entry.seconds = INI::scanReal(ini->getNextToken());
	((Config *)instance)->weapons.push_back(entry);
}

const FieldParse configFields[] =
{
	{ "Class",				parseClass,					nullptr, 0 },
	{ "Weapon",				parseWeapon,				nullptr, 0 },
	{ "MaxDamageRadius",	INI::parseReal,				nullptr, offsetof(Config, maxDamageRadius) },
	{ "MaxSlowDown",		INI::parseReal,				nullptr, offsetof(Config, maxSlowDown) },
	{ "DamagePerSecond",	INI::parsePercentToReal,	nullptr, offsetof(Config, damagePerSecond) },
	{ nullptr, nullptr, nullptr, 0 }
};

const OwnLimit *ownLimit(const Config &config, const WeaponTemplate *weapon)
{
	for (const OwnLimit &entry : config.weapons)
	{
		if (entry.weapon.compareNoCase(weapon->getName()) == 0)
			return &entry;
	}
	return nullptr;
}

WeaponFacts factsOf(const Config &config, const WeaponTemplate *weapon)
{
	const WeaponBonus noBonus;

	WeaponFacts facts;
	facts.cycle.minDelay = weapon->getMinDelayBetweenShots();
	facts.cycle.maxDelay = weapon->getMaxDelayBetweenShots();
	facts.cycle.clipSize = weapon->getClipSize();
	facts.cycle.clipReload = weapon->getClipReloadTime(noBonus);
	if (const WeaponBonusSet *extra = weapon->getExtraBonus())
	{
		WeaponBonus spunUp;
		extra->appendBonuses(1 << WEAPONBONUSCONDITION_CONTINUOUS_FIRE_FAST, spunUp);
		facts.spinUp = spunUp.getField(WeaponBonus::RATE_OF_FIRE);
	}

	const DamageType type = weapon->getDamageType();
	if (type >= 0 && type < DAMAGE_NUM_TYPES)
		facts.classSeconds = config.classSeconds[type];
	if (const OwnLimit *own = ownLimit(config, weapon))
	{
		facts.hasOwnLine = true;
		facts.ownSeconds = own->seconds;
	}
	facts.dealsDamage = weapon->getPrimaryDamage(noBonus) > 0 || weapon->getSecondaryDamage(noBonus) > 0;
	facts.damageRadius = max(weapon->getPrimaryDamageRadius(noBonus), weapon->getSecondaryDamageRadius(noBonus));
	facts.scatterRadius = weapon->getScatterRadius();
	facts.damageAtSelf = weapon->getDamageDealtAtSelfPosition();
	return facts;
}

Scaling lookUpScaling(const WeaponTemplate *weapon)
{
	const Config &config = storage();
	Rules rules;
	rules.maxDamageRadius = config.maxDamageRadius;
	rules.maxSlowDown = config.maxSlowDown;
	rules.damagePerSecond = config.damagePerSecond;

	const WeaponFacts facts = factsOf(config, weapon);
	const Scaling scaling = scalingFor(facts, rules, LOGICFRAMES_PER_SECOND);
	if (scaling.changes())
	{
		const double rate = topShotsPerSecond(facts, LOGICFRAMES_PER_SECOND);
		CoH::log("FireRate: %s %.2f shots/s, now %.2f: x%.2f slower, x%.2f damage", weapon->getName().str(),
			rate, rate / scaling.slowDown, scaling.slowDown, scaling.damage);
	}
	return scaling;
}

} // namespace

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

const Scaling &weaponScaling(const WeaponTemplate *weapon)
{
	static const Scaling none;
	if (weapon == nullptr)
		return none;

	std::unordered_map<const WeaponTemplate *, Scaling> &known = knownWeapons();
	auto found = known.find(weapon);
	if (found == known.end())
		found = known.emplace(weapon, lookUpScaling(weapon)).first;
	return found->second;
}

} // namespace CoH::FireRate
