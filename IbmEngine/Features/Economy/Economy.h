// IBM Engine: economy, the functions the engine's hooks call. With the Economy feature on, units
// are scarcer and tougher: the IbmEconomy INI block (in the Economy pack) multiplies the cost,
// build time, health, weapon damage, rate of fire and speed of whole kinds of units (vehicles,
// aircraft) and of single units, so each one is worth more and lives longer.

#pragma once

class INI;
class LocomotorSet;
class Object;
class ThingTemplate;
class WeaponTemplate;

namespace IbmEngine
{

enum EconomyStat
{
	EconomyStat_Cost,
	EconomyStat_BuildTime,
	EconomyStat_Health,
	EconomyStat_Damage,
	EconomyStat_Speed,
	EconomyStat_RateOfFire,
	EconomyStat_VsInfantry,	///< damage multiplier against infantry

	EconomyStat_Count
};

// The multiplier for one stat of a unit; 1 when the feature is off or the unit isn't listed.
// Hooks: ThingTemplate::calcCostToBuild and calcTimeToBuild, ActiveBody's constructor and
// Weapon::computeBonus.
Real getEconomyScale(const ThingTemplate *tmpl, EconomyStat stat);

// WeaponTemplate::dealDamageInternal, before each victim takes damage: the attacker's VsInfantry
// multiplier when the victim is infantry, else 1. A "VsInfantryByDamage = <DamageType> <percent>"
// line replaces it for the weapons of that type, for attackers that have one (tank guns, whose
// shells retail infantry armor cuts to 10%). attacker may be null (it died).
Real getEconomyDamageScaleAgainst(const WeaponTemplate *weapon, const Object *attacker, const Object *victim);

// WeaponTemplate::getDelayBetweenShots and getClipReloadTime: a weapon with a "Reload = <Weapon>
// <seconds>" line fires once per that many seconds: a weapon with no clip waits that long between
// shots, a weapon with a clip (one shell, or a burst) takes that long to reload it. Returns the time
// in logic frames, or -1 when the weapon has no line or the feature is off. Weapons with a line skip
// the unit's RateOfFire multiplier; veterancy and upgrades still speed them up.
Int getEconomyReloadFrames(const WeaponTemplate *weapon);

// AIUpdateInterface::chooseLocomotorSetExplicit, after a new set is built: caps each locomotor's
// speed at its normal speed times the unit's Speed multiplier.
void applyEconomySpeed(const Object *obj, LocomotorSet &set);

// INI block "IbmEconomy": the multipliers, read from the Economy pack at game start.
void parseEconomyDefinition(INI *ini);

} // namespace IbmEngine
