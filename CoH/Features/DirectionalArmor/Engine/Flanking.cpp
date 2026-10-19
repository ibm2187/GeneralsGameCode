// CoH DirectionalArmor: the skirmish AI flanks. Every FlankCheckEvery seconds, each AI vehicle fighting
// an armored vehicle from in front of it, where its gun would bounce off, drives to the target's side
// (Domain/Flank.h) and attacks again from there. The AI's own orders always win: a flanker that gets
// another order lets the flank go. A flanker rests a while before it flanks again.
//
// The flanks in progress aren't saved: after a load, a flanker finishes its drive and the AI picks
// its fights again.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"

#include "Kernel/Log.h"
#include "Kernel/Objects.h"
#include "Kernel/Players.h"
#include "Kernel/Time.h"

#include "Features/DirectionalArmor/Engine/Config.h"
#include "Features/DirectionalArmor/Engine/Internal.h"

#include <map>

namespace CoH::DirectionalArmor
{

namespace
{

constexpr Real ArrivedDistance = 25.0f;	///< this close to the spot is there
constexpr Real SameGoalDistance = 5.0f;	///< a goal this far from the spot is someone else's order

struct Flank
{
	ObjectID target = INVALID_ID;
	Coord3D spot;
	UnsignedInt giveUpFrame = 0;
};

// By object ID, so every machine walks them in the same order.
std::map<ObjectID, Flank> &flanks()
{
	static std::map<ObjectID, Flank> theFlanks;
	return theFlanks;
}

std::map<ObjectID, UnsignedInt> &restingUntil()
{
	static std::map<ObjectID, UnsignedInt> theResting;
	return theResting;
}

int &flanksStarted()
{
	static int count = 0;
	return count;
}

Flat flat(const Coord3D &v)
{
	return { v.x, v.y };
}

Real flatDistance(const Coord3D &a, const Coord3D &b)
{
	const Real dx = a.x - b.x;
	const Real dy = a.y - b.y;
	return std::sqrt(dx * dx + dy * dy);
}

void rest(ObjectID id)
{
	restingUntil()[id] = now() + config().flankRestFrames;
}

bool isResting(ObjectID id)
{
	const auto found = restingUntil().find(id);
	return found != restingUntil().end() && found->second > now();
}

// The flanks under way: attack again on arrival or time out; let go of ones the AI ordered otherwise.
void followFlanks()
{
	for (auto it = flanks().begin(); it != flanks().end();)
	{
		const auto &[id, flank] = *it;
		Object *flanker = findLiving(id);
		Object *target = findLiving(flank.target);
		AIUpdateInterface *ai = flanker != nullptr ? flanker->getAI() : nullptr;
		if (ai == nullptr || target == nullptr)
		{
			it = flanks().erase(it);
			continue;
		}

		const Coord3D *goal = ai->getGoalPosition();
		const bool reordered = ai->isAttacking() || (goal != nullptr && !ai->isIdle()
			&& flatDistance(*goal, flank.spot) > SameGoalDistance);
		const bool arrived = ai->isIdle() || flatDistance(*flanker->getPosition(), flank.spot) <= ArrivedDistance;
		if (!reordered && (arrived || now() >= flank.giveUpFrame))
			ai->aiAttackObject(target, NO_MAX_SHOTS_LIMIT, CMD_FROM_AI);
		if (reordered || arrived || now() >= flank.giveUpFrame)
		{
			rest(id);
			it = flanks().erase(it);
			continue;
		}
		++it;
	}
}

// Whether the vehicle should drive around its target, and where to.
std::optional<Flat> flankFor(Object &vehicle, const Object &target)
{
	const ArmorValues *armor = armorOf(target);
	const Weapon *weapon = vehicle.getCurrentWeapon();
	if (armor == nullptr || weapon == nullptr || weapon->getTemplate() == nullptr)
		return std::nullopt;
	const Penetration *penetration = penetrationOf(*weapon->getTemplate());
	if (penetration == nullptr)
		return std::nullopt;

	const Config &c = config();
	const FlankSituation situation = {
		.target = flat(*target.getPosition()),
		.targetFacing = flat(*target.getUnitDirectionVector2D()),
		.attacker = flat(*vehicle.getPosition()),
		.arcs = c.arcs,
		.armor = *armor,
		.penetration = *penetration,
		.range = weapon->getAttackRange(&vehicle),
		.hits = { .deflectedShare = c.deflectedDamage, .sideDamage = c.sideDamage },
	};
	return flankSpot(situation, c.flank);
}

void startFlanks(Player &side)
{
	const ObjectFilter vehicles = { .anyKind = kinds(KINDOF_VEHICLE), .noKind = kinds(KINDOF_AIRCRAFT, KINDOF_IMMOBILE) };
	for (Object *vehicle : objectsOf(side, vehicles))
	{
		const ObjectID id = vehicle->getID();
		AIUpdateInterface *ai = vehicle->getAI();
		if (ai == nullptr || vehicle->isContained() || vehicle->isDisabled() || flanks().contains(id) || isResting(id))
			continue;
		const Object *target = ai->isAttacking() ? ai->getCurrentVictim() : nullptr;
		if (target == nullptr || target->isEffectivelyDead() || !isEnemy(*vehicle, *target))
			continue;

		const std::optional<Flat> spot = flankFor(*vehicle, *target);
		if (!spot)
			continue;
		Coord3D where = *target->getPosition();
		where.x = spot->x;
		where.y = spot->y;
		ai->aiMoveToPosition(&where, CMD_FROM_AI);
		flanks()[id] = { target->getID(), where, now() + config().flankTimeoutFrames };
		++flanksStarted();
	}
}

void forgetRested()
{
	for (auto it = restingUntil().begin(); it != restingUntil().end();)
		it = it->second <= now() ? restingUntil().erase(it) : std::next(it);
}

} // namespace

void flankWithAI()
{
	const Config &c = config();
	if (!c.flanking || !isEvery(now(), c.flankCheckFrames))
		return;
	followFlanks();
	forgetRested();
	for (Player *side : aiSides())
		startFlanks(*side);
}

void logFlanking()
{
	if (isLogging() && flanksStarted() > 0)
		log("DirectionalArmor: AI flanks started %d", flanksStarted());
}

void forgetFlanking()
{
	flanks().clear();
	restingUntil().clear();
	flanksStarted() = 0;
}

} // namespace CoH::DirectionalArmor
