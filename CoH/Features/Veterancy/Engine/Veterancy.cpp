// CoH Veterancy: the slice's declaration and the rank bonuses: accuracy through Accuracy's modifier
// stack, damage taken, reload, speed and sight through the hub, and an upgrade at a rank. Experience
// and squads are in Experience.cpp. See CoH/Features/Veterancy/README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GlobalData.h"
#include "Common/ThingTemplate.h"
#include "Common/Upgrade.h"
#include "GameLogic/Damage.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"

#include "Kernel/Kernel.h"
#include "Kernel/Log.h"

#include "Features/Accuracy/Accuracy.h"
#include "Features/Veterancy/Domain/Experience.h"
#include "Features/Veterancy/Engine/Config.h"
#include "Features/Veterancy/Engine/Internal.h"
#include "Features/Veterancy/Veterancy.h"

namespace CoH::Veterancy
{

namespace
{

const Feature *&theFeature()
{
	static const Feature *feature = nullptr;
	return feature;
}

// Stands in for Accuracy's built-in "Veterancy" (+15% a rank) while this flag is on, so the two
// never stack.
Accuracy::HitModifier rankAccuracy(const Accuracy::Shot &shot)
{
	return Accuracy::scaleChance(bonusOf(shot.shooter).accuracy);
}

void scaleDamageTaken(const Object *victim, const DamageInfo * /*damage*/, float &amount)
{
	amount *= bonusOf(victim).damageTaken;
}

void scaleSpeed(const Object *unit, float &speed)
{
	speed *= bonusOf(unit).speed;
}

WeaponBonusConditionType engineCondition(VeterancyLevel level)
{
	switch (level)
	{
		case LEVEL_VETERAN: return WEAPONBONUSCONDITION_VETERAN;
		case LEVEL_ELITE: return WEAPONBONUSCONDITION_ELITE;
		case LEVEL_HEROIC: return WEAPONBONUSCONDITION_HERO;
		default: return WEAPONBONUSCONDITION_INVALID;
	}
}

// The rate of fire the engine's own bonuses give the rank: GameData's and the weapon's extra ones.
Real engineRankRateOfFire(const WeaponTemplate *weapon, WeaponBonusConditionType condition)
{
	if (condition == WEAPONBONUSCONDITION_INVALID)
		return 1.0f;
	const WeaponBonusConditionFlags flags = (WeaponBonusConditionFlags)1 << condition;
	WeaponBonus rank;
	rank.clear();
	if (TheGlobalData->m_weaponBonusSet)
		TheGlobalData->m_weaponBonusSet->appendBonuses(flags, rank);
	if (weapon != nullptr && weapon->getExtraBonus() != nullptr)
		weapon->getExtraBonus()->appendBonuses(flags, rank);
	return rank.getField(WeaponBonus::RATE_OF_FIRE);
}

// The rank's reload in place of the engine's rate of fire for the rank. A projectile going off
// (the source is then the projectile) has no rank, and its rate of fire doesn't matter.
void swapReload(const Object *source, const WeaponTemplate *weapon, WeaponBonus &bonus)
{
	if (source == nullptr || source->getExperienceTracker() == nullptr)
		return;
	const RankBonus &rank = bonusOf(source);
	const WeaponBonusConditionType condition = engineCondition(source->getVeterancyLevel());
	if (condition != WEAPONBONUSCONDITION_INVALID
		&& (source->getWeaponBonusCondition() & ((WeaponBonusConditionFlags)1 << condition)) == 0)
		return;
	const Real engineRank = engineRankRateOfFire(weapon, condition);
	if (rank.reload == engineRank)
		return;
	bonus.setField(WeaponBonus::RATE_OF_FIRE,
		swapRankBonus(bonus.getField(WeaponBonus::RATE_OF_FIRE), engineRank, rank.reload));
}

void scaleSight(Object *unit, float from, float to)
{
	const Real change = rankChange(from, to);
	if (change == 1.0f)
		return;
	unit->setVisionRange(unit->getVisionRange() * change);
	unit->setShroudClearingRange(unit->getShroudClearingRange() * change);
}

// The upgrade a rank gives, once, on reaching it. Only object upgrades (drones, an Overlord's
// addon): a player upgrade would reach every unit of its player.
void giveRankUpgrade(Object *unit, const RankBonus &rank)
{
	if (rank.upgrade.empty())
		return;
	const UpgradeTemplate *upgrade = TheUpgradeCenter->findUpgrade(rank.upgrade.c_str());
	if (upgrade == nullptr || upgrade->getUpgradeType() != UPGRADE_TYPE_OBJECT)
	{
		CoH::log("Veterancy: %s is not an object upgrade; %s gets nothing", rank.upgrade.c_str(),
			unit->getTemplate()->getName().str());
		return;
	}
	if (unit->hasUpgrade(upgrade))
		return;
	unit->giveUpgrade(upgrade);
	CoH::log("Veterancy: %s %u gained %s", unit->getTemplate()->getName().str(), (unsigned)unit->getID(),
		rank.upgrade.c_str());
}

void onVeterancyChanged(Object *unit, int oldLevel, int newLevel)
{
	const RankTable &table = tableOf(unit->getTemplate());
	const RankBonus &from = table[static_cast<std::size_t>(rankOf(oldLevel))];
	const RankBonus &to = table[static_cast<std::size_t>(rankOf(newLevel))];
	CoH::log("Veterancy: %s %u rank %d to %d (experience %d)", unit->getTemplate()->getName().str(),
		(unsigned)unit->getID(), oldLevel, newLevel, unit->getExperienceTracker()->getCurrentExperience());
	scaleSight(unit, from.sight, to.sight);
	for (int level = oldLevel + 1; level <= newLevel; ++level)
		giveRankUpgrade(unit, table[static_cast<std::size_t>(rankOf(level))]);
	onUnitRankChanged(unit);
}

// A regular's own sight bonus, if the pack gives one: there is no rank change to scale it on. A
// loaded object's saved sight replaces this one.
void onObjectCreated(Object *object)
{
	if (object->getExperienceTracker() != nullptr)
		scaleSight(object, 1.0f, tableOf(object->getTemplate())[0].sight);
}

void onObjectDestroyed(Object *object)
{
	forgetUnit(object);
}

void onGameEnd()
{
	forgetExperience();
}

void init(Feature &self)
{
	theFeature() = &self;
	registerIniBlock("CoHVeterancy", parseConfigBlock);
	registerSaveData(self, "Veterancy", saveData());

	Accuracy::addHitModifier(self, "Veterancy", Accuracy::ModifierOrder::Veterancy, rankAccuracy);
	Hooks::damageReceived().subscribe(self, scaleDamageTaken);
	Hooks::weaponBonus().subscribe(self, swapReload);
	Hooks::unitSpeed().subscribe(self, scaleSpeed);
	Hooks::veterancyChanged().subscribe(self, onVeterancyChanged);

	Hooks::damageDealt().subscribe(self, onDamageDealt);
	Hooks::killScored().subscribe(self, onKillScored);
	Hooks::logicFrame().subscribe(self, shareSquadExperience);
	Hooks::objectCreated().subscribe(self, onObjectCreated);
	Hooks::objectDestroyed().subscribe(self, onObjectDestroyed);
	Hooks::gameEnd().subscribe(self, onGameEnd);

	Hooks::commandTooltip().subscribe(self, addRanksToTooltip);
}

void reset()
{
	forgetExperience();
	resetConfig();
}

} // namespace

bool isActive()
{
	return theFeature() != nullptr && theFeature()->isActive();
}

float suppressionResistance(const Object *unit)
{
	return isActive() ? bonusOf(unit).suppressionResistance : 0.0f;
}

float accuracyMultiplier(const Object *unit)
{
	return isActive() ? bonusOf(unit).accuracy : 1.0f;
}

float damageTakenMultiplier(const Object *unit)
{
	return isActive() ? bonusOf(unit).damageTaken : 1.0f;
}

} // namespace CoH::Veterancy

COH_FEATURE(Veterancy, CoH::Veterancy::init, CoH::Veterancy::reset)
