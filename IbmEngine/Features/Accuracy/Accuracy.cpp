// IBM Engine: accuracy. See Accuracy.h and README.md.
//
// Hit chance = base accuracy of the weapon's kind (or its own line in the INI)
//            x veterancy x range x shooter moving x target moving x target in cover,
// kept between MinChance and MaxChance. Rolls use the game logic's random numbers, so every PC
// and every replay sees the same hits and misses.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameCommon.h"
#include "Common/INI.h"
#include "Common/NameKeyGenerator.h"
#include "Common/RandomValue.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Weapon.h"
#include "GameLogic/Module/PhysicsUpdate.h"

#include "Features/Accuracy/Accuracy.h"
#include "Features/FeatureFlags/FeatureFlags.h"

namespace
{

//-------------------------------------------------------------------------------------------------
// The IbmAccuracy block. Defaults match the pack, so a missing block plays the same.
struct AccuracySettings
{
	Real rifle;					///< SMALL_ARMS
	Real machineGun;			///< GATTLING, COMANCHE_VULCAN
	Real sniper;				///< SNIPER, KILLPILOT
	Real tankGun;				///< ARMOR_PIERCING
	Real rocket;				///< INFANTRY_MISSILE, JET_MISSILES, STEALTHJET_MISSILES, small EXPLOSION projectiles
	Real rocketMaxRadius;		///< EXPLOSION projectiles up to this damage radius are rockets; bigger is artillery
	Real veteran;
	Real elite;
	Real heroic;
	Real minChance;
	Real maxChance;
	Real rangeFalloffStart;		///< share of the weapon's range shot at full accuracy
	Real longRange;				///< multiplier at the weapon's full range
	Real shooterMoving;
	Real targetMoving;
	Real movingSpeed;			///< speed (world units per frame) above which a unit counts as moving
	Real cover;
	Real coverRadius;			///< infantry this close to a building or bush is in cover
	Real missDistance;			///< how far beyond the target's edge a miss lands, at most

	AccuracySettings()
	{
		rifle = 0.65f;
		machineGun = 0.50f;
		sniper = 0.95f;
		tankGun = 0.75f;
		rocket = 0.80f;
		rocketMaxRadius = 10.0f;
		veteran = 1.15f;
		elite = 1.30f;
		heroic = 1.45f;
		minChance = 0.05f;
		maxChance = 0.95f;
		rangeFalloffStart = 0.5f;
		longRange = 0.75f;
		shooterMoving = 0.80f;
		targetMoving = 0.85f;
		movingSpeed = 0.5f;
		cover = 0.70f;
		coverRadius = 25.0f;
		missDistance = 15.0f;
	}
};

AccuracySettings s_settings;
std::map<NameKeyType, Real> s_weaponAccuracy;	///< "Weapon = Name 70%" lines

//-------------------------------------------------------------------------------------------------
// "Weapon = RangerAdvancedCombatRifle 70%": that weapon's own base accuracy (100% never misses).
void parseWeaponAccuracy(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	const char *name = ini->getNextToken();
	const Real accuracy = INI::scanPercentToReal(ini->getNextToken());
	s_weaponAccuracy[NAMEKEY(name)] = accuracy;
}

const FieldParse s_fieldParse[] =
{
	{ "Rifle",				INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, rifle) },
	{ "MachineGun",			INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, machineGun) },
	{ "Sniper",				INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, sniper) },
	{ "TankGun",			INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, tankGun) },
	{ "Rocket",				INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, rocket) },
	{ "RocketMaxRadius",	INI::parseReal,				nullptr, offsetof(AccuracySettings, rocketMaxRadius) },
	{ "Weapon",				parseWeaponAccuracy,		nullptr, 0 },
	{ "Veteran",			INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, veteran) },
	{ "Elite",				INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, elite) },
	{ "Heroic",				INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, heroic) },
	{ "MinChance",			INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, minChance) },
	{ "MaxChance",			INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, maxChance) },
	{ "RangeFalloffStart",	INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, rangeFalloffStart) },
	{ "LongRange",			INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, longRange) },
	{ "ShooterMoving",		INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, shooterMoving) },
	{ "TargetMoving",		INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, targetMoving) },
	{ "MovingSpeed",		INI::parseReal,				nullptr, offsetof(AccuracySettings, movingSpeed) },
	{ "Cover",				INI::parsePercentToReal,	nullptr, offsetof(AccuracySettings, cover) },
	{ "CoverRadius",		INI::parseReal,				nullptr, offsetof(AccuracySettings, coverRadius) },
	{ "MissDistance",		INI::parseReal,				nullptr, offsetof(AccuracySettings, missDistance) },
	{ nullptr, nullptr, nullptr, 0 }
};

//-------------------------------------------------------------------------------------------------
// The weapon's hit chance before modifiers, or a negative number if it never misses.
Real getBaseAccuracy(const WeaponTemplate *weapon, const WeaponBonus &bonus)
{
	std::map<NameKeyType, Real>::const_iterator it = s_weaponAccuracy.find(NAMEKEY(weapon->getName()));
	if (it != s_weaponAccuracy.end())
		return it->second >= 1.0f ? -1.0f : it->second;

	switch (weapon->getDamageType())
	{
		case DAMAGE_SMALL_ARMS:
			return s_settings.rifle;

		case DAMAGE_GATTLING:
		case DAMAGE_COMANCHE_VULCAN:
			return s_settings.machineGun;

		case DAMAGE_SNIPER:
		case DAMAGE_KILLPILOT:
			return s_settings.sniper;

		case DAMAGE_ARMOR_PIERCING:
			return s_settings.tankGun;

		case DAMAGE_INFANTRY_MISSILE:
		case DAMAGE_JET_MISSILES:
		case DAMAGE_STEALTHJET_MISSILES:
			return s_settings.rocket;

		case DAMAGE_EXPLOSION:
			// Missiles and rockets; shells with a big blast are artillery, which doesn't roll.
			if (weapon->getProjectileTemplate() != nullptr && weapon->getPrimaryDamageRadius(bonus) <= s_settings.rocketMaxRadius)
				return s_settings.rocket;
			return -1.0f;

		default:
			return -1.0f; // flame, poison, lasers, bombs, melee and the rest
	}
}

//-------------------------------------------------------------------------------------------------
Real getVeterancyMultiplier(VeterancyLevel level)
{
	switch (level)
	{
		case LEVEL_VETERAN: return s_settings.veteran;
		case LEVEL_ELITE:	return s_settings.elite;
		case LEVEL_HEROIC:	return s_settings.heroic;
		default:			return 1.0f;
	}
}

//-------------------------------------------------------------------------------------------------
// A soldier riding in something moves with it.
Bool isMoving(const Object *obj)
{
	const Object *container = obj->getContainedBy();
	if (container != nullptr)
		obj = container;

	const PhysicsBehavior *physics = obj->getPhysics();
	return physics != nullptr && physics->getVelocityMagnitude() > s_settings.movingSpeed;
}

//-------------------------------------------------------------------------------------------------
class PartitionFilterCover : public PartitionFilter
{
public:
	PartitionFilterCover(const Object *soldier) : m_soldier(soldier) { }

	virtual Bool allow(Object *other) override
	{
		if (other == m_soldier || other->isEffectivelyDead() || other == m_soldier->getContainedBy())
			return FALSE;
		return other->isKindOf(KINDOF_STRUCTURE) || other->isKindOf(KINDOF_SHRUBBERY);
	}

#if defined(RTS_DEBUG)
	virtual const char *debugGetName() override { return "PartitionFilterCover"; }
#endif

private:
	const Object *m_soldier;
};

// Cover is looked up at most twice a second per soldier: machine guns fire many times a second.
// The cache only depends on the game's own state, so every PC fills it the same way.
const UnsignedInt COVER_CACHE_FRAMES = 15;
struct CoverEntry
{
	UnsignedInt frame;
	Bool inCover;
};
std::map<ObjectID, CoverEntry> s_coverCache;
UnsignedInt s_coverCacheFrame = 0;

Bool isInCover(const Object *soldier)
{
	const UnsignedInt now = TheGameLogic->getFrame();
	if (now < s_coverCacheFrame || s_coverCache.size() > 1024)
		s_coverCache.clear(); // a new game, or too many entries to keep
	s_coverCacheFrame = now;

	std::map<ObjectID, CoverEntry>::iterator it = s_coverCache.find(soldier->getID());
	if (it != s_coverCache.end() && now - it->second.frame < COVER_CACHE_FRAMES)
		return it->second.inCover;

	PartitionFilterCover filterCover(soldier);
	PartitionFilter *filters[] = { &filterCover, nullptr };
	const Bool inCover = ThePartitionManager->getClosestObject(soldier->getPosition(), s_settings.coverRadius, FROM_BOUNDINGSPHERE_2D, filters) != nullptr;

	CoverEntry entry;
	entry.frame = now;
	entry.inCover = inCover;
	s_coverCache[soldier->getID()] = entry;
	return inCover;
}

} // namespace

//-------------------------------------------------------------------------------------------------
void IbmEngine::parseAccuracyDefinition(INI *ini)
{
	// Each game's pack sets everything again.
	s_settings = AccuracySettings();
	s_weaponAccuracy.clear();
	ini->initFromINI(&s_settings, s_fieldParse);
}

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::rollAccuracyMiss(const Object *source, const Object *victim, const WeaponTemplate *weapon,
	const WeaponBonus &bonus, Coord3D &aimPos)
{
	if (!isFeatureActive(Feature_Accuracy) || source == nullptr || victim == nullptr || weapon == nullptr)
		return FALSE;

	// Only units roll: buildings are too big to miss, and missiles, mines and props aren't aimed at.
	if (victim->isKindOf(KINDOF_STRUCTURE)
		|| !(victim->isKindOf(KINDOF_INFANTRY) || victim->isKindOf(KINDOF_VEHICLE) || victim->isKindOf(KINDOF_AIRCRAFT)))
		return FALSE;

	const Real base = getBaseAccuracy(weapon, bonus);
	if (base < 0.0f)
		return FALSE;

	Real chance = base * getVeterancyMultiplier(source->getVeterancyLevel());

	// Range: full accuracy up to RangeFalloffStart of the weapon's range, then down to LongRange.
	const Real range = weapon->getAttackRange(bonus);
	if (range > 0.0f)
	{
		const Real dist = sqrtf(ThePartitionManager->getDistanceSquared(source, victim, FROM_BOUNDINGSPHERE_2D));
		const Real start = range * s_settings.rangeFalloffStart;
		if (dist > start && range > start)
		{
			const Real t = min(1.0f, (dist - start) / (range - start));
			chance *= 1.0f + (s_settings.longRange - 1.0f) * t;
		}
	}

	if (isMoving(source))
		chance *= s_settings.shooterMoving;
	if (isMoving(victim))
		chance *= s_settings.targetMoving;
	if (victim->isKindOf(KINDOF_INFANTRY) && isInCover(victim))
		chance *= s_settings.cover;

	chance = max(s_settings.minChance, min(s_settings.maxChance, chance));

	const Bool hit = GameLogicRandomValueReal(0.0f, 1.0f) < chance;

#if defined(RTS_DEBUG)
	static Int s_rolls = 0;
	static Int s_hits = 0;
	++s_rolls;
	if (hit)
		++s_hits;
	if (s_rolls % 500 == 0)
		DEBUG_LOG(("IbmEngine accuracy: %d hits of %d rolls (%d%%)", s_hits, s_rolls, s_hits * 100 / s_rolls));
#endif

	if (hit)
		return FALSE;

	// The miss lands on the ground beside the target, or in the air beside a flying one.
	const Real angle = GameLogicRandomValueReal(0.0f, 2.0f * PI);
	const Real dist = victim->getGeometryInfo().getBoundingCircleRadius() + GameLogicRandomValueReal(0.5f, 1.0f) * s_settings.missDistance;
	const Coord3D *pos = victim->getPosition();
	aimPos.x = pos->x + dist * Cos(angle);
	aimPos.y = pos->y + dist * Sin(angle);
	aimPos.z = victim->isAboveTerrain() ? pos->z : TheTerrainLogic->getLayerHeight(aimPos.x, aimPos.y, victim->getLayer());
	return TRUE;
}
