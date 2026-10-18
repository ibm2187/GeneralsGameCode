// CoH Squads: a squad takes the room of one unit in a transport or a garrison (Domain/Room.h), and
// shows as one passenger: its first rider there gets the button and the pip, and lets the squad out.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/Object.h"

#include "Features/Squads/Domain/Room.h"
#include "Features/Squads/Engine/Internal.h"

namespace CoH::Squads
{

namespace
{

// The room one object takes on its own: a transport's slots, or one place in a garrison.
int roomOf(const Object *object, bool garrison)
{
	return garrison ? 1 : object->getTransportSlotCount();
}

Rider riderOf(const Object *object, bool garrison)
{
	Rider rider;
	rider.squad = book().squadOf((MemberId)object->getID());
	rider.room = roomOf(object, garrison);
	return rider;
}

} // namespace

void onContainRoom(const Object *container, const Object *rider, int &used, int &needed)
{
	if (!isSquadsGame() || container == nullptr || rider == nullptr)
		return;
	const ContainModuleInterface *contain = container->getContain();
	if (contain == nullptr)
		return;
	const bool garrison = contain->isGarrisonable();

	std::vector<Rider> inside;
	int roomOnTheirOwn = 0;
	if (const ContainedItemsList *items = contain->getContainedItemsList())
	{
		for (const Object *object : *items)
		{
			inside.push_back(riderOf(object, garrison));
			roomOnTheirOwn += inside.back().room;
		}
	}

	// The engine counted every rider; give back what the squads' later members took.
	used -= roomOnTheirOwn - roomUsed(inside);
	Rider newcomer = riderOf(rider, garrison);
	newcomer.room = needed;
	needed = roomNeeded(newcomer, inside);
}

bool isRiderShown(const Object * /*container*/, const Object *rider)
{
	const Squad *squad = squadOfObject(rider);
	// Its mates are the riders of the container it is in (an Overlord's bunker, not the Overlord).
	const Object *inside = rider != nullptr ? rider->getContainedBy() : nullptr;
	if (squad == nullptr || inside == nullptr || inside->getContain() == nullptr)
		return true;

	std::vector<MemberId> riders;
	if (const ContainedItemsList *items = inside->getContain()->getContainedItemsList())
	{
		for (const Object *object : *items)
			riders.push_back((MemberId)object->getID());
	}
	const MemberId speaker = squad->leaderAmong(riders);
	return speaker == 0 || speaker == (MemberId)rider->getID();
}

} // namespace CoH::Squads
