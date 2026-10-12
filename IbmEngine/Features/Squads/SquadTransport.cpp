// IBM Engine: squads in transports and buildings. In retail, mob leaders and members can't enter
// anything. Here a squad may enter a container when all its members fit: the members go in one by
// one like any infantry, and once they are all inside, IbmSquadUpdate parks the invisible leader in
// the container too. When one member leaves, the rest of the squad follows.
//
// Transports count squads, not soldiers: a transport with N slots carries N / SlotsPerSquad squads
// (at least one; the IbmSquadTransport block can set a transport's own number), and each squad
// inside takes one share of the slots whatever its size. Buildings and tunnels count soldiers.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/ActionManager.h"
#include "Common/INI.h"
#include "Common/NameKeyGenerator.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/ContainModule.h"

#include "Features/FeatureFlags/FeatureFlags.h"
#include "Features/Squads/Squads.h"
#include "Features/Squads/SquadsInternal.h"
#include "Features/Squads/SquadUpdate.h"

namespace
{

//-------------------------------------------------------------------------------------------------
// The IbmSquadTransport block. Defaults match the pack.
struct SquadTransportSettings
{
	Int slotsPerSquad;							///< a transport carries its slots / this many squads
	std::map<NameKeyType, Int> squadsCarried;	///< "Transport = Name 3" lines

	SquadTransportSettings() : slotsPerSquad(4) { }
};

SquadTransportSettings s_transportSettings;

void parseTransportSquads(INI *ini, void * /*instance*/, void * /*store*/, const void * /*userData*/)
{
	const char *name = ini->getNextToken();
	s_transportSettings.squadsCarried[NAMEKEY(name)] = max(1, INI::scanInt(ini->getNextToken()));
}

const FieldParse s_transportFieldParse[] =
{
	{ "SlotsPerSquad",	INI::parseInt,			nullptr, offsetof(SquadTransportSettings, slotsPerSquad) },
	{ "Transport",		parseTransportSquads,	nullptr, 0 },
	{ nullptr, nullptr, nullptr, 0 }
};

//-------------------------------------------------------------------------------------------------
// The slots one squad takes in this transport.
Int getSquadShare(const Object *transport, const ContainModuleInterface *contain)
{
	const Int slots = contain->getContainMax();
	Int squads = max(1, slots / max(1, s_transportSettings.slotsPerSquad));

	std::map<NameKeyType, Int>::const_iterator it = s_transportSettings.squadsCarried.find(NAMEKEY(transport->getTemplate()->getName()));
	if (it != s_transportSettings.squadsCarried.end())
		squads = it->second;

	return max(1, slots / squads);
}

//-------------------------------------------------------------------------------------------------
// Does the transport hold a member of this squad (other than exclude)?
Bool holdsSquadMember(const ContainModuleInterface *contain, const Object *leader, const Object *exclude)
{
	const ContainedItemsList *items = contain->getContainedItemsList();
	if (items == nullptr)
		return FALSE;

	for (ContainedItemsList::const_iterator it = items->begin(); it != items->end(); ++it)
	{
		if (*it != exclude && IbmEngine::getSquadLeaderOf(*it) == leader)
			return TRUE;
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
// Transports count squads; buildings, tunnels, hospitals, bikes and parachutes count soldiers.
Bool countsSquads(const ContainModuleInterface *contain)
{
	return !contain->isGarrisonable() && !contain->isTunnelContain() && !contain->isHealContain()
		&& !contain->isRiderChangeContain() && !contain->isSpecialZeroSlotContainer();
}

} // namespace

//-------------------------------------------------------------------------------------------------
void IbmEngine::parseSquadTransportDefinition(INI *ini)
{
	// Each game's pack sets everything again.
	s_transportSettings = SquadTransportSettings();
	ini->initFromINI(&s_transportSettings, s_transportFieldParse);
}

//-------------------------------------------------------------------------------------------------
Int IbmEngine::getTransportRiderSlots(const Object *transport, const ContainModuleInterface *contain, const Object *rider, Int slots)
{
	if (!isFeatureActive(Feature_Squads))
		return slots;

	const Object *leader = getSquadLeaderOf(rider);
	if (leader == nullptr)
		return slots;

	return holdsSquadMember(contain, leader, rider) ? 0 : getSquadShare(transport, contain);
}

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::recountTransportSlots(const Object *transport, const ContainModuleInterface *contain, const Object *boarding, Int &extraSlots)
{
	if (!isFeatureActive(Feature_Squads))
		return FALSE;

	std::vector<const Object *> squadsInside;
	Int riders = 0;
	Int slotsUsed = 0;
	auto count = [&](const Object *rider)
	{
		++riders;
		const Object *leader = getSquadLeaderOf(rider);
		if (leader == nullptr)
		{
			slotsUsed += rider->getTransportSlotCount();
		}
		else if (std::find(squadsInside.begin(), squadsInside.end(), leader) == squadsInside.end())
		{
			squadsInside.push_back(leader);
			slotsUsed += getSquadShare(transport, contain);
		}
	};

	const ContainedItemsList *items = contain->getContainedItemsList();
	if (items != nullptr)
	{
		for (ContainedItemsList::const_iterator it = items->begin(); it != items->end(); ++it)
			count(*it);
	}
	if (boarding != nullptr)
		count(boarding);

	// Below zero when a squad has more soldiers than its share: they ride on fewer slots.
	extraSlots = slotsUsed - riders;
	return TRUE;
}

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
	const Object *firstEntering = nullptr;
	forEachSquadMember(obj, [&](Object *member)
	{
		if (!canMemberEnter(member, objectToEnter, cmdSource))
		{
			allCanEnter = FALSE;
			return;
		}
		++entering;
		if (firstEntering == nullptr)
			firstEntering = member;
		// Garrisons count soldiers, other containers slots.
		slotsNeeded += contain->isGarrisonable() ? 1 : max(1, member->getTransportSlotCount());
	});

	if (entering == 0 || (!allCanEnter && !healOnly))
		return SQUAD_ENTER_NO;

	if (mode == CHECK_CAPACITY && contain->getContainMax() >= 0)
	{
		if (countsSquads(contain))
		{
			// The transport knows the squad's share (an Overlord asks its bunker).
			if (!contain->isValidContainerFor(firstEntering, TRUE))
				return SQUAD_ENTER_NO;
		}
		else
		{
			const Int slotsFree = contain->getContainMax() - (Int)contain->getContainCount() - contain->getExtraSlotsInUse();
			if (slotsNeeded > slotsFree)
				return SQUAD_ENTER_NO;
		}
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
