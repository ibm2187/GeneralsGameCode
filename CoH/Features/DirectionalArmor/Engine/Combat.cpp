// CoH DirectionalArmor: each victim's damage, from the hub's weaponDamage (before the engine's armor).
// Vehicles roll penetration by the side the shot hits; infantry takes more from vehicles' weapons.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/Damage.h"
#include "GameLogic/Module/BehaviorModule.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/Weapon.h"

#include "Kernel/Log.h"
#include "Kernel/Objects.h"
#include "Kernel/Random.h"

#include "Features/FireRate/FireRate.h"

#include "Features/DirectionalArmor/Domain/Blast.h"
#include "Features/DirectionalArmor/Engine/Config.h"
#include "Features/DirectionalArmor/Engine/Internal.h"

#include <array>
#include <cmath>
#include <map>
#include <string>
#include <vector>

namespace CoH::DirectionalArmor
{

namespace
{

struct Count
{
	int rolls = 0;
	int deflected = 0;
};

struct Tally
{
	std::array<Count, 3> bySide;			///< front, side, rear
	std::map<std::string, Count> byWeapon;	///< for the log only: never read by game logic
	int infantryHits = 0;
	int blasts = 0;				///< shells that went off with a blast
	int blastVictims = 0;		///< soldiers beside the target the blasts hurt
};

Tally &tally()
{
	static Tally theTally;
	return theTally;
}

Flat flat(const Coord3D &v)
{
	return { v.x, v.y };
}

Flat from(const Coord3D &to, const Coord3D &origin)
{
	return { origin.x - to.x, origin.y - to.y };
}

// Where the shot came from: the shooter (a projectile's launcher), else what dealt the damage (the
// projectile), else where it went off.
const Coord3D &shotOrigin(const Hooks::WeaponDamage &hit, const Object *shooter)
{
	if (shooter != nullptr)
		return *shooter->getPosition();
	if (hit.source != nullptr)
		return *hit.source->getPosition();
	return *hit.position;
}

bool isWeaponHit(const Hooks::WeaponDamage &hit)
{
	const DamageInfoInput &in = hit.damage->in;
	return hit.weapon != nullptr && in.m_amount > 0.0f && in.m_amount < HUGE_DAMAGE_AMOUNT
		&& IsHealthDamagingDamage(in.m_damageType) && in.m_damageType != DAMAGE_UNRESISTABLE;
}

// Tank guns and vehicle machine guns against soldiers (InfantryDamage lines). A soldier a shell with a
// blast kills is thrown by it.
void scaleAgainstInfantry(Hooks::WeaponDamage &hit, const Object *shooter)
{
	const Config &c = config();
	if (shooter == nullptr || !hit.victim->isKindOf(KINDOF_INFANTRY))
		return;
	if (!shooter->isAnyKindOf(c.infantryDamageFrom) || shooter->isAnyKindOf(c.infantryDamageNotFrom))
		return;

	if (blastOf(*hit.weapon) != nullptr)
		hit.damage->in.m_deathType = DEATH_EXPLODED;
	const Real factor = infantryDamageFactor(hit.damage->in.m_damageType);
	if (factor == 1.0f)
		return;
	hit.damage->in.m_amount *= factor;
	++tally().infantryHits;
}

// The roll against the side the shot hits. It draws one synced number for each hit on an armored
// vehicle by a weapon with penetration, and none otherwise.
void rollPenetration(Hooks::WeaponDamage &hit, const Object *shooter)
{
	const Config &c = config();
	const ArmorValues *armor = armorOf(*hit.victim);
	if (armor == nullptr)
		return;
	const Penetration *penetration = penetrationOf(*hit.weapon);
	if (penetration == nullptr)
		return;
	const WeaponBonus noBonus;
	if (max(hit.weapon->getPrimaryDamageRadius(noBonus), hit.weapon->getSecondaryDamageRadius(noBonus)) > c.areaRadius)
		return;	// artillery, bombs, super weapons: area damage always gets through

	const Coord3D &victimAt = *hit.victim->getPosition();
	const Flat towardsShot = from(victimAt, shotOrigin(hit, shooter));
	const Side side = sideHit(flat(*hit.victim->getUnitDirectionVector2D()), towardsShot, c.arcs);
	const float distance = std::sqrt(towardsShot.x * towardsShot.x + towardsShot.y * towardsShot.y);
	const float pushing = penetrationAt(*penetration, distance, hit.weapon->getAttackRange(noBonus));

	const HitRules rules = { .deflectedShare = c.deflectedDamage, .sideDamage = c.sideDamage };
	const Hit result = resolveHit(side, *armor, pushing, rules, [](float p) { return CoH::chance(p); });
	hit.damage->in.m_amount *= result.damageScale;

	Tally &t = tally();
	Count &bySide = t.bySide[(int)side];
	Count &byWeapon = t.byWeapon[hit.weapon->getName().str()];
	++bySide.rolls;
	++byWeapon.rolls;
	if (!result.penetrated)
	{
		++bySide.deflected;
		++byWeapon.deflected;
		showDeflection(*hit.victim, towardsShot, pushing);
	}
}

// Who fired the shot: a projectile's launcher, or the shooter itself (as dealDamageInternal credits it).
ObjectID launcherOf(const Object *source)
{
	if (source == nullptr)
		return INVALID_ID;
	if (source->isKindOf(KINDOF_PROJECTILE))
	{
		for (BehaviorModule **module = source->getBehaviorModules(); *module != nullptr; ++module)
		{
			if (const ProjectileUpdateInterface *projectile = (*module)->getProjectileUpdateInterface())
				return projectile->projectileGetLauncherID();
		}
	}
	return source->getID();
}

struct Caught
{
	ObjectID soldier;
	Real damage;
};

// The soldiers the blast reaches beyond the shell's own radius, found as the engine finds a blast's
// victims (FROM_BOUNDINGSPHERE_3D, in the partition's order).
std::vector<Caught> soldiersCaught(const BlastShell &shell, const Coord3D &position, Real primaryRadius)
{
	std::vector<Caught> caught;
	SimpleObjectIterator *iter = ThePartitionManager->iterateObjectsInRange(&position, shell.radius, FROM_BOUNDINGSPHERE_3D);
	MemoryPoolObjectHolder hold(iter);
	Real distanceSquared = 0.0f;
	for (Object *object = iter->firstWithNumeric(&distanceSquared); object != nullptr;
		object = iter->nextWithNumeric(&distanceSquared))
	{
		if (!object->isKindOf(KINDOF_INFANTRY) || object->isEffectivelyDead())
			continue;
		const std::optional<Real> damage =
			blastDamage(shell, config().infantryBlast, std::sqrt(distanceSquared), primaryRadius);
		if (damage)
			caught.push_back({ object->getID(), *damage });
	}
	return caught;
}

} // namespace

void detonateBlast(const Object *source, const WeaponTemplate *weapon, const Coord3D &position, float /*radius*/)
{
	const BlastShell *shell = weapon != nullptr ? blastOf(*weapon) : nullptr;
	if (shell == nullptr)
		return;
	const WeaponBonus noBonus;
	const std::vector<Caught> caught = soldiersCaught(*shell, position, weapon->getPrimaryDamageRadius(noBonus));
	++tally().blasts;

	// FireRate fires slow guns less often and harder by the same factor; the blast keeps up with it.
	const Real fireRateDamage = FireRate::slowDown(weapon);
	const ObjectID launcher = launcherOf(source);
	const Player *owner = source != nullptr ? source->getControllingPlayer() : nullptr;
	for (const Caught &hit : caught)
	{
		Object *soldier = findLiving(hit.soldier);
		if (soldier == nullptr)
			continue;
		DamageInfo damage;
		damage.in.m_damageType = weapon->getDamageType();
		damage.in.m_deathType = DEATH_EXPLODED;
		damage.in.m_sourceID = launcher;
		damage.in.m_sourcePlayerMask = owner != nullptr ? owner->getPlayerMask() : 0;
		damage.in.m_damageStatusType = weapon->getDamageStatusType();
		damage.in.m_amount = hit.damage * fireRateDamage;
		Hooks::onWeaponDamage(source, weapon, &position, soldier, &damage);	// Cover, then InfantryDamage
		soldier->attemptDamage(&damage);
		++tally().blastVictims;
	}
}

void dealWeaponDamage(Hooks::WeaponDamage &hit)
{
	if (!isWeaponHit(hit))
		return;
	const Object *shooter = findObject(hit.damage->in.m_sourceID);
	scaleAgainstInfantry(hit, shooter);
	rollPenetration(hit, shooter);
}

int blastsSoFar()
{
	return tally().blasts;
}

void logTally()
{
	const Tally &t = tally();
	if (!isLogging() || (t.bySide[0].rolls + t.bySide[1].rolls + t.bySide[2].rolls + t.infantryHits + t.blasts) == 0)
		return;
	for (const Side side : { Side::Front, Side::Side, Side::Rear })
	{
		const Count &count = t.bySide[(int)side];
		log("DirectionalArmor: %s hits %d, deflected %d", sideName(side), count.rolls, count.deflected);
	}
	for (const auto &[weapon, count] : t.byWeapon)
		log("DirectionalArmor:   %s %d hits, %d deflected", weapon.c_str(), count.rolls, count.deflected);
	log("DirectionalArmor: infantry hit harder by vehicles %d times", t.infantryHits);
	log("DirectionalArmor: shell blasts %d, soldiers caught beside the target %d", t.blasts, t.blastVictims);
}

void forgetTally()
{
	tally() = Tally();
}

} // namespace CoH::DirectionalArmor
