// IBM Engine: attacking a squad, like in Company of Heroes. An attack on one trooper is an attack on
// his whole squad: the attackers spread their fire over its troopers and, when one dies, move on to
// the next until the squad is gone. This uses the engine's own team attack (AI_ATTACK_SQUAD), aimed
// at the squad's troopers, with its own choice of victim.
//
// Force attacks do the same (AI_FORCE_ATTACK_SQUAD). Skill shots (Kell's snipe, Burton's knife)
// still hit exactly the trooper they were aimed at.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/Squad.h"
#include "GameLogic/Weapon.h"

#include "Features/Squads/Squads.h"
#include "Features/Squads/SquadsInternal.h"

//-------------------------------------------------------------------------------------------------
// The squad leader a victim belongs to: its own leader, or itself if it is one.
static const Object *findVictimSquad(const Object *victim)
{
	if (IbmEngine::isSquadLeader(victim))
		return victim;
	return IbmEngine::getSquadLeaderOf(victim);
}

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::isSquadMemberOrLeader(const Object *obj)
{
	return findVictimSquad(obj) != nullptr;
}

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::makeSquadTarget(const Object *attacker, const Object *victim, Squad &squad)
{
	if (attacker == nullptr || victim == nullptr)
		return FALSE;

	const Object *leader = findVictimSquad(victim);
	if (leader == nullptr)
		return FALSE;

	// Aircraft have attack runs of their own; a squad's own members follow their leader.
	if (attacker->isKindOf(KINDOF_AIRCRAFT) || attacker == leader || getSquadLeaderOf(attacker) == leader)
		return FALSE;

	// A skill shot aimed at one man: a weapon locked for a button that fires it.
	WeaponSlotType slot;
	if (attacker->isCurWeaponLocked() && attacker->getCurrentWeapon(&slot) != nullptr && hasFireWeaponButton(attacker, slot))
		return FALSE;

	squad.clearSquad();
	forEachSquadMember(leader, [&](Object *member)
	{
		squad.addObject(member);
	});

	return squad.getSizeOfGroup() > 0;
}

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::chooseSquadVictim(const Object *attacker, Squad *squad, CommandSourceType cmdSource, Bool forceAttack, Object *&victim)
{
	if (squad == nullptr)
		return FALSE;

	const VecObjectPtr &objects = squad->getLiveObjects();
	if (objects.empty())
		return FALSE;

	// Only squads made by makeSquadTarget: every trooper belongs to the same squad.
	const Object *leader = getSquadLeaderOf(objects.front());
	if (leader == nullptr)
		return FALSE;

	std::vector<Object *> targets;
	for (VecObjectPtr::const_iterator it = objects.begin(); it != objects.end(); ++it)
	{
		if (getSquadLeaderOf(*it) != leader)
			return FALSE;

		// Skip troopers this attacker can't hit, such as hidden ones or a man inside a building.
		const CanAttackResult result = attacker->getAbleToAttackSpecificObject(forceAttack ? ATTACK_NEW_TARGET_FORCED : ATTACK_NEW_TARGET, *it, cmdSource);
		if (result == ATTACKRESULT_POSSIBLE || result == ATTACKRESULT_POSSIBLE_AFTER_MOVING)
			targets.push_back(*it);
	}

	// Spread the attackers over the troopers. IDs are the same on every machine, so this stays in sync.
	victim = targets.empty() ? nullptr : targets[attacker->getID() % targets.size()];
	return TRUE;
}
