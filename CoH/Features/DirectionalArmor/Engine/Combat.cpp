// CoH DirectionalArmor: each victim's damage, from the hub's weaponDamage (before the engine's armor).
// Vehicles roll penetration by the side the shot hits; infantry takes more from vehicles' weapons.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/ThingTemplate.h"
#include "GameLogic/Damage.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"

#include "Kernel/Log.h"
#include "Kernel/Objects.h"
#include "Kernel/Random.h"

#include "Features/DirectionalArmor/Domain/Blast.h"
#include "Features/DirectionalArmor/Engine/Config.h"
#include "Features/DirectionalArmor/Engine/Internal.h"

#include <array>
#include <cmath>
#include <map>
#include <string>

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

// How far the victim's body is from where the damage went off, as the engine measures a blast's reach
// (FROM_BOUNDINGSPHERE_3D in dealDamageInternal).
Real blastDistance(const Hooks::WeaponDamage &hit)
{
	if (hit.position == nullptr)
		return 0.0f;
	const Coord3D &at = *hit.victim->getPosition();
	const Real dx = at.x - hit.position->x;
	const Real dy = at.y - hit.position->y;
	const Real dz = at.z - hit.position->z;
	return max(0.0f, std::sqrt(dx * dx + dy * dy + dz * dz) - hit.victim->getGeometryInfo().getBoundingSphereRadius());
}

// Tank guns and vehicle machine guns against soldiers (InfantryDamage lines), and a shell's blast
// falling off from its middle (InfantryBlast).
void scaleAgainstInfantry(Hooks::WeaponDamage &hit, const Object *shooter)
{
	const Config &c = config();
	if (shooter == nullptr || !hit.victim->isKindOf(KINDOF_INFANTRY))
		return;
	if (!shooter->isAnyKindOf(c.infantryDamageFrom) || shooter->isAnyKindOf(c.infantryDamageNotFrom))
		return;

	const WeaponBonus noBonus;
	const Real blast = blastScale(c.infantryBlast, blastDistance(hit), hit.weapon->getPrimaryDamageRadius(noBonus),
		hit.weapon->getSecondaryDamageRadius(noBonus));
	const Real factor = infantryDamageFactor(hit.damage->in.m_damageType) * blast;
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

} // namespace

void dealWeaponDamage(Hooks::WeaponDamage &hit)
{
	if (!isWeaponHit(hit))
		return;
	const Object *shooter = findObject(hit.damage->in.m_sourceID);
	scaleAgainstInfantry(hit, shooter);
	rollPenetration(hit, shooter);
}

void logTally()
{
	const Tally &t = tally();
	if (!isLogging() || (t.bySide[0].rolls + t.bySide[1].rolls + t.bySide[2].rolls + t.infantryHits) == 0)
		return;
	for (const Side side : { Side::Front, Side::Side, Side::Rear })
	{
		const Count &count = t.bySide[(int)side];
		log("DirectionalArmor: %s hits %d, deflected %d", sideName(side), count.rolls, count.deflected);
	}
	for (const auto &[weapon, count] : t.byWeapon)
		log("DirectionalArmor:   %s %d hits, %d deflected", weapon.c_str(), count.rolls, count.deflected);
	log("DirectionalArmor: infantry hit harder by vehicles %d times", t.infantryHits);
}

void forgetTally()
{
	tally() = Tally();
}

} // namespace CoH::DirectionalArmor
