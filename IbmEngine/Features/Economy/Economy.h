// IBM Engine: economy, the functions the engine's hooks call. With the Economy feature on, units
// are scarcer and tougher: the IbmEconomy INI block (in the Economy pack) multiplies the cost,
// build time, health, weapon damage and speed of whole kinds of units (vehicles, aircraft) and of
// single units, so each one is worth more and lives longer.

#pragma once

class INI;
class LocomotorSet;
class Object;
class ThingTemplate;

namespace IbmEngine
{

enum EconomyStat
{
	EconomyStat_Cost,
	EconomyStat_BuildTime,
	EconomyStat_Health,
	EconomyStat_Damage,
	EconomyStat_Speed,

	EconomyStat_Count
};

// The multiplier for one stat of a unit; 1 when the feature is off or the unit isn't listed.
// Hooks: ThingTemplate::calcCostToBuild and calcTimeToBuild, ActiveBody's constructor and
// Weapon::computeBonus.
Real getEconomyScale(const ThingTemplate *tmpl, EconomyStat stat);

// AIUpdateInterface::chooseLocomotorSetExplicit, after a new set is built: caps each locomotor's
// speed at its normal speed times the unit's Speed multiplier.
void applyEconomySpeed(const Object *obj, LocomotorSet &set);

// INI block "IbmEconomy": the multipliers, read from the Economy pack at game start.
void parseEconomyDefinition(INI *ini);

} // namespace IbmEngine
