// IBM Engine: squads in transports and buildings. In retail, mob leaders and members can't enter
// anything. Here a squad may enter a container when all its members fit: the members go in one by
// one like any infantry, and once they are all inside, IbmSquadUpdate parks the invisible leader in
// the container too. When one member leaves, the rest of the squad follows.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/ActionManager.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/ContainModule.h"

#include "Features/Squads/Squads.h"
#include "Features/Squads/SquadUpdate.h"

//-------------------------------------------------------------------------------------------------
// Can this member enter the container on its own? (Capacity is checked for the whole squad.)
static Bool canMemberEnter(const Object *member, const Object *container, CommandSourceType cmdSource)
{
	return !member->isContained()
		&& TheActionManager->canEnterObject(member, container, cmdSource, DONT_CHECK_CAPACITY);
}

//-------------------------------------------------------------------------------------------------
IbmEngine::SquadEnterCheck IbmEngine::checkSquadEnter(const Object *obj, const Object *objectToEnter, CommandSourceType cmdSource, CanEnterType mode)
{
	// Cargo planes and the like stay closed to everyone.
	if (objectToEnter->isKindOf(KINDOF_IGNORED_IN_GUI))
		return SQUAD_ENTER_NOT_A_SQUAD;

	if (getSquadLeaderOf(obj) != nullptr)
		return SQUAD_ENTER_CHECK_AS_USUAL;

	if (!isSquadLeader(obj))
		return SQUAD_ENTER_NOT_A_SQUAD;

	// Combat drops and taking over empty vehicles are for single soldiers.
	if (mode == COMBATDROP_INTO || objectToEnter->isDisabledByType(DISABLED_UNMANNED))
		return SQUAD_ENTER_NO;

	ContainModuleInterface *contain = objectToEnter->getContain();
	if (contain == nullptr)
		return SQUAD_ENTER_NO;

	// A healing container (a hospital) takes only the wounded; everywhere else all must fit.
	const Bool healOnly = contain->isHealContain();
	Bool allCanEnter = TRUE;
	Int entering = 0;
	Int slotsNeeded = 0;
	forEachSquadMember(obj, [&](Object *member)
	{
		if (!canMemberEnter(member, objectToEnter, cmdSource))
		{
			allCanEnter = FALSE;
			return;
		}
		++entering;
		// Garrisons count soldiers, transports count slots.
		slotsNeeded += contain->isGarrisonable() ? 1 : max(1, member->getTransportSlotCount());
	});

	if (entering == 0 || (!allCanEnter && !healOnly))
		return SQUAD_ENTER_NO;

	if (mode == CHECK_CAPACITY && contain->getContainMax() >= 0)
	{
		const Int slotsFree = contain->getContainMax() - (Int)contain->getContainCount() - contain->getExtraSlotsInUse();
		if (slotsNeeded > slotsFree)
			return SQUAD_ENTER_NO;
	}

	return SQUAD_ENTER_YES;
}

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::onSquadEnter(Object *obj, Object *container, CommandSourceType cmdSource)
{
	if (container == nullptr)
		return FALSE;

	// A member told to enter on its own: let it, and keep the mob code from pulling it back.
	if (Object *leader = getSquadLeaderOf(obj))
	{
		getSquad(leader)->setMemberOnOwnOrder(obj->getID());
		return FALSE;
	}

	IbmSquadUpdate *squad = getSquad(obj);
	if (squad == nullptr)
		return FALSE;

	if (!TheActionManager->canEnterObject(obj, container, cmdSource, CHECK_CAPACITY))
		return TRUE;

	forEachSquadMember(obj, [&](Object *member)
	{
		AIUpdateInterface *ai = member->getAI();
		if (ai != nullptr && canMemberEnter(member, container, cmdSource))
		{
			ai->aiEnter(container, cmdSource);
			squad->setMemberOnOwnOrder(member->getID());
		}
	});

	// The leader walks along; it parks inside once every member is in.
	AIUpdateInterface *ai = obj->getAI();
	if (ai != nullptr)
		ai->aiMoveToPosition(container->getPosition(), cmdSource);

	return TRUE;
}

//-------------------------------------------------------------------------------------------------
void IbmEngine::onSquadMemberExit(Object *member, Object *container, CommandSourceType cmdSource)
{
	// Each member we order out calls back in here.
	static Bool s_orderingSquadOut = FALSE;
	if (s_orderingSquadOut || container == nullptr)
		return;

	Object *leader = getSquadLeaderOf(member);
	const ContainModuleInterface *contain = container->getContain();
	if (leader == nullptr || contain == nullptr)
		return;

	s_orderingSquadOut = TRUE;
	forEachSquadMember(leader, [&](Object *other)
	{
		// Asks the container, not the member: a tunnel network holds everyone who entered any of
		// its tunnels, and they may all leave through this one.
		AIUpdateInterface *ai = other->getAI();
		if (other != member && ai != nullptr && contain->isContained(other))
			ai->aiExit(container, cmdSource);
	});
	s_orderingSquadOut = FALSE;
}
