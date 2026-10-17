// CoH Squads: the answers of the public header, Squads.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Object.h"

#include "Features/Squads/Engine/Config.h"
#include "Features/Squads/Engine/Internal.h"
#include "Features/Squads/Squads.h"

namespace CoH::Squads
{

bool isActive()
{
	return isSquadsGame();
}

int squadOf(const Object *object)
{
	const Squad *squad = squadOfObject(object);
	return squad != nullptr ? squad->id() : 0;
}

std::vector<Object *> members(int squad)
{
	const Squad *found = isSquadsGame() ? book().find(squad) : nullptr;
	return found != nullptr ? livingMembers(*found) : std::vector<Object *>();
}

Object *leader(int squad)
{
	const std::vector<Object *> living = members(squad);
	return living.empty() ? nullptr : living.front();
}

bool countsAsUnit(const Object *object)
{
	const int squad = squadOf(object);
	return squad == 0 || leader(squad) == object;
}

int slotCount(int squad)
{
	const Squad *found = isSquadsGame() ? book().find(squad) : nullptr;
	return found != nullptr ? found->slotCount() : 0;
}

int missingMembers(int squad)
{
	const Squad *found = isSquadsGame() ? book().find(squad) : nullptr;
	return found != nullptr ? found->missingCount() : 0;
}

const ThingTemplate *unitOfSlot(int squad, int slot)
{
	const Squad *found = isSquadsGame() ? book().find(squad) : nullptr;
	if (found == nullptr)
		return nullptr;
	const ThingTemplate *ordered = TheThingFactory->findTemplate(found->unit().c_str());
	const SquadType *type = squadTypeOf(ordered);
	if (type == nullptr)
		return nullptr;
	// The ordered unit's own kind (a general's version) for its slots, the escorts by name after them.
	if (slot >= 0 && slot < type->size)
		return ordered;
	const std::string escort = type->unitOfSlot(slot);
	return escort.empty() ? nullptr : TheThingFactory->findTemplate(escort.c_str());
}

std::vector<int> emptySlots(int squad)
{
	std::vector<int> slots;
	const Squad *found = isSquadsGame() ? book().find(squad) : nullptr;
	for (int slot = 0; found != nullptr && slot < found->slotCount(); ++slot)
	{
		if (memberObject(found->memberIn(slot)) == nullptr)
			slots.push_back(slot);
	}
	return slots;
}

bool addMember(int squad, Object *member)
{
	if (!isSquadsGame() || member == nullptr)
		return false;
	const BodyModuleInterface *body = member->getBodyModule();
	return book().join(squad, (MemberId)member->getID(), body != nullptr ? body->getMaxHealth() : 0.0f);
}

void moveTo(int squad, const Coord3D &goal)
{
	std::vector<Object *> movers;
	for (Object *member : members(squad))
	{
		if (member->getContainedBy() == nullptr)
			movers.push_back(member);
	}
	sendInFormation(movers, goal, CMD_FROM_AI, false, 0);
}

bool formationSlot(const Object *member, Coord3D *position)
{
	if (squadOf(member) == 0)
		return false;
	return lastSlotPosition((MemberId)member->getID(), position);
}

bool isSquadUnit(const ThingTemplate *unit)
{
	return squadTypeOf(unit) != nullptr;
}

int buildCostOf(const ThingTemplate *unit)
{
	return rawSquadCost(unit);
}

} // namespace CoH::Squads
