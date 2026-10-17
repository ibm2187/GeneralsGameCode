// CoH Accuracy: the hit roll. The hub's weaponFiringAtObject asks before a weapon fires at an object;
// a miss sends the shot to a spot near the target, where the engine treats it as a shot at the
// ground. Rolls draw synced random numbers, so every machine and replay agrees.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/PhysicsUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Weapon.h"

#include "Features/Accuracy/Engine/Config.h"
#include "Features/Accuracy/Engine/Internal.h"

namespace CoH::Accuracy
{

namespace
{

Tally &storedTally()
{
	static Tally theTally;
	return theTally;
}

// ---- The built-in modifiers, with this game's rules ----------------------------------------------

HitModifier shooterMoving(const Shot &shot) { return shooterMovingModifier(config().rules, shot); }
HitModifier targetMoving(const Shot &shot) { return targetMovingModifier(config().rules, shot); }
HitModifier targetSize(const Shot &shot) { return targetSizeModifier(config().rules, shot); }
HitModifier veterancy(const Shot &shot) { return veterancyModifier(config().rules, shot); }

// ---- What the shot looks like -------------------------------------------------------------------

// A soldier riding in a vehicle, or firing from one, moves with it.
bool isMoving(const Object *object)
{
	const Object *container = object->getContainedBy();
	if (container != nullptr)
		object = container;

	const PhysicsBehavior *physics = object->getPhysics();
	return physics != nullptr && physics->getVelocityMagnitude() > config().movingSpeed;
}

// False for what isn't rolled against: mines, projectiles, props.
bool findTargetSize(const Object *target, TargetSize &size)
{
	if (target->isKindOf(KINDOF_STRUCTURE))
		size = TargetSize::Building;
	else if (target->isKindOf(KINDOF_INFANTRY))
		size = TargetSize::Infantry;
	else if (target->isKindOf(KINDOF_AIRCRAFT) && target->isSignificantlyAboveTerrain())
		size = TargetSize::Aircraft;
	else if (target->isKindOf(KINDOF_VEHICLE) || target->isKindOf(KINDOF_AIRCRAFT))
		size = TargetSize::Vehicle;
	else
		return false;
	return true;
}

Shot describeShot(const Hooks::WeaponShot &fired, TargetSize size)
{
	Shot shot;
	shot.shooter = fired.source;
	shot.target = fired.victim;
	shot.weapon = fired.weapon;
	shot.distance = sqrtf(ThePartitionManager->getDistanceSquared(fired.source, fired.victim, FROM_BOUNDINGSPHERE_2D));
	shot.range = fired.weapon->getAttackRange(*fired.bonus);
	shot.shooterMoving = isMoving(fired.source);
	shot.targetMoving = isMoving(fired.victim);
	shot.targetSize = size;
	shot.targetRadius = fired.victim->getGeometryInfo().getBoundingCircleRadius();
	shot.veterancy = (int)fired.source->getVeterancyLevel();
	return shot;
}

// On the ground beside the target, or in the air beside a flying one.
Coord3D findMissSpot(const Object *target)
{
	const Real angle = GameLogicRandomValueReal(0.0f, 2.0f * PI);
	const Real spread = GameLogicRandomValueReal(0.0f, 1.0f);
	const Offset offset = missOffset(config().rules, target->getGeometryInfo().getBoundingCircleRadius(), angle, spread);

	Coord3D spot = *target->getPosition();
	spot.x += offset.x;
	spot.y += offset.y;
	if (!target->isSignificantlyAboveTerrain())
		spot.z = TheTerrainLogic->getLayerHeight(spot.x, spot.y, target->getLayer());
	return spot;
}

bool rollsHit(float chance, TargetSize size)
{
	const bool hit = isHit(chance, GameLogicRandomValue(0, RollSides - 1));
	Count &counted = storedTally().bySize[(int)size];
	++counted.rolls;
	if (hit)
		++counted.hits;
	return hit;
}

} // namespace

ModifierStack &modifiers()
{
	static ModifierStack theStack;
	return theStack;
}

void addBuiltInModifiers(const Feature &self)
{
	ModifierStack &stack = modifiers();
	stack.addBuiltIn(self, "ShooterMoving", ModifierOrder::Movement, shooterMoving);
	stack.addBuiltIn(self, "TargetMoving", ModifierOrder::Movement, targetMoving);
	stack.addBuiltIn(self, "TargetSize", ModifierOrder::TargetSize, targetSize);
	stack.addBuiltIn(self, "Veterancy", ModifierOrder::Veterancy, veterancy);
}

void rollShot(Hooks::WeaponShot &fired)
{
	if (fired.retargeted)
		return;

	const RangeProfile &profile = weaponProfile(fired.weapon);
	TargetSize size;
	if (!profile.isSet() || !findTargetSize(fired.victim, size))
		return;

	// The weapon's accuracy at this range is the base; the modifiers do the rest.
	const Shot shot = describeShot(fired, size);
	const float base = accuracyAt(profile, shot.distance, shot.range);
	const float chance = hitChance(config().rules, base, modifiers(), shot);
	const bool hit = needsRoll(chance) ? rollsHit(chance, size) : chance > 0.0f;
	if (hit)
		return;

	*fired.spot = findMissSpot(fired.victim);
	fired.retargeted = true;

	// A projectile goes off where it lands, with its own effects; a bullet shows a puff of dirt.
	if (fired.weapon->getProjectileTemplate() == nullptr)
		showMissPuff(*fired.spot);
}

const Tally &tally()
{
	return storedTally();
}

void forgetTally()
{
	storedTally() = Tally();
}

} // namespace CoH::Accuracy
