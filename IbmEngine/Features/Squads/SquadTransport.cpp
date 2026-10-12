// IBM Engine: squads in transports and buildings. In retail, mob leaders and members can't enter
// anything. Here a squad may enter a container when all its members fit: the members go in one by
// one like any infantry, and once they are all inside, IbmSquadUpdate parks the invisible leader in
// the container too. When one member leaves, the rest of the squad follows.
//
// Transports count squads, not soldiers: a transport with N slots has N / SlotsPerSquad places (at
// least one; the IbmSquadTransport block can set a transport's own number), and a squad takes one
// place whatever its size. The engine still counts slots: a place is a share of them, and the slots
// no place can use (8 slots for 3 places leave 2) count as taken once a squad is inside. Single
// soldiers and vehicles keep their own slots, so a Troop Crawler still comes with its 8 Red Guards.
// The passenger panel and the pips over the transport show one button per squad or single rider,
// plus one per free place. Buildings and tunnels count soldiers.

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
	Int slotsPerSquad;							///< a transport has its slots / this many places
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
// Transports count places; buildings, tunnels, hospitals, bikes and parachutes count soldiers.
Bool countsSquads(const ContainModuleInterface *contain)
{
	return !contain->isGarrisonable() && !contain->isTunnelContain() && !contain->isHealContain()
		&& !contain->isRiderChangeContain() && !contain->isSpecialZeroSlotContainer();
}

//-------------------------------------------------------------------------------------------------
// How a transport's slots split into places.
struct TransportPlaces
{
	Int slots;		///< the transport's slots
	Int places;		///< squads it carries
	Int share;		///< slots per place
	Int unused;		///< slots no place can use

	TransportPlaces(const Object *transport, const ContainModuleInterface *contain)
	{
		slots = max(1, contain->getContainMax());
		places = max(1, slots / max(1, s_transportSettings.slotsPerSquad));

		std::map<NameKeyType, Int>::const_iterator it = s_transportSettings.squadsCarried.find(NAMEKEY(transport->getTemplate()->getName()));
		if (it != s_transportSettings.squadsCarried.end())
			places = min(slots, it->second);

		share = slots / places;
		unused = slots - share * places;
	}
};

//-------------------------------------------------------------------------------------------------
// Does the transport hold a member of this squad (other than exclude)? Of any squad, for nullptr.
Bool holdsSquadMember(const ContainModuleInterface *contain, const Object *leader, const Object *exclude)
{
	const ContainedItemsList *items = contain->getContainedItemsList();
	if (items == nullptr)
		return FALSE;

	for (ContainedItemsList::const_iterator it = items->begin(); it != items->end(); ++it)
	{
		if (*it == exclude)
			continue;

		const Object *itsLeader = IbmEngine::getSquadLeaderOf(*it);
		if (itsLeader != nullptr && (leader == nullptr || itsLeader == leader))
			return TRUE;
	}
	return FALSE;
}

//-------------------------------------------------------------------------------------------------
// Counts what a transport holds: every rider in its list, plus boarding if not nullptr.
struct TransportLoad
{
	Int slotsUsed;		///< slots taken: a place per squad, the unused slots once a squad is in, singles' own
	Int soldiers;		///< everyone in the list, plus boarding
	Int riders;			///< squads and single riders: one button each
	Int infantryRiders;	///< of those, squads and single soldiers

	TransportLoad(const TransportPlaces &places, const ContainModuleInterface *contain, const Object *boarding)
		: slotsUsed(0), soldiers(0), riders(0), infantryRiders(0)
	{
		std::vector<const Object *> squadsInside;
		auto count = [&](const Object *rider)
		{
			++soldiers;
			const Object *leader = IbmEngine::getSquadLeaderOf(rider);
			if (leader != nullptr)
			{
				if (std::find(squadsInside.begin(), squadsInside.end(), leader) != squadsInside.end())
					return;
				squadsInside.push_back(leader);
				slotsUsed += places.share;
			}
			else
			{
				slotsUsed += rider->getTransportSlotCount();
			}

			++riders;
			if (rider->isKindOf(KINDOF_INFANTRY))
				++infantryRiders;
		};

		const ContainedItemsList *items = contain->getContainedItemsList();
		if (items != nullptr)
		{
			for (ContainedItemsList::const_iterator it = items->begin(); it != items->end(); ++it)
				count(*it);
		}
		if (boarding != nullptr)
			count(boarding);

		if (!squadsInside.empty())
			slotsUsed += places.unused;
	}
};

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
	if (!isFeatureActive(Feature_Squads) || !countsSquads(contain))
		return slots;

	const Object *leader = getSquadLeaderOf(rider);
	if (leader == nullptr)
		return slots;

	if (holdsSquadMember(contain, leader, rider))
		return 0; // the squad has its place already

	// The first squad aboard also takes the slots no place can use.
	const TransportPlaces places(transport, contain);
	return places.share + (holdsSquadMember(contain, nullptr, rider) ? 0 : places.unused);
}

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::recountTransportSlots(const Object *transport, const ContainModuleInterface *contain, const Object *boarding, Int &extraSlots)
{
	if (!isFeatureActive(Feature_Squads) || !countsSquads(contain))
		return FALSE;

	const TransportPlaces places(transport, contain);
	const TransportLoad load(places, contain, boarding);

	// Below zero when a squad has more soldiers than its place has slots.
	extraSlots = load.slotsUsed - load.soldiers;
	return TRUE;
}

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::getSquadTransportButtons(const Object *transport, Int &buttons, Int &riders, Int &infantryRiders)
{
	const ContainModuleInterface *contain = transport != nullptr ? transport->getContain() : nullptr;
	if (!isFeatureActive(Feature_Squads) || contain == nullptr || !countsSquads(contain) || contain->getContainMax() <= 0)
		return FALSE;

	const TransportPlaces places(transport, contain);
	const TransportLoad load(places, contain, nullptr);

	// A free place needs the slots of a squad (and the unused ones, before the first squad).
	Int freeSlots = places.slots - load.slotsUsed;
	if (!holdsSquadMember(contain, nullptr, nullptr))
		freeSlots -= places.unused;

	buttons = load.riders + max(0, freeSlots) / places.share;
	riders = load.riders;
	infantryRiders = load.infantryRiders;
	return TRUE;
}

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::showSquadInventoryButton(const Object *transport, const Object *rider, const Image *&image)
{
	const ContainModuleInterface *contain = transport != nullptr ? transport->getContain() : nullptr;
	if (!isFeatureActive(Feature_Squads) || contain == nullptr || !countsSquads(contain))
		return TRUE;

	const Object *leader = getSquadLeaderOf(rider);
	if (leader == nullptr)
		return TRUE;

	// Only the squad's first member in the list gets a button.
	const ContainedItemsList *items = contain->getContainedItemsList();
	if (items != nullptr)
	{
		for (ContainedItemsList::const_iterator it = items->begin(); it != items->end() && *it != rider; ++it)
		{
			if (getSquadLeaderOf(*it) == leader)
				return FALSE;
		}
	}

	if (const Image *squadImage = leader->getTemplate()->getButtonImage())
		image = squadImage;
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
