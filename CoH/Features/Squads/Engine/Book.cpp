// CoH Squads: the squads of the game being played, kept by member object ID.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/ThingTemplate.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"

#include "Features/Squads/Engine/Config.h"
#include "Features/Squads/Engine/Internal.h"

namespace CoH::Squads
{

namespace
{

struct GameState
{
	bool playing = false;
	SquadBook book;
};

GameState &state()
{
	static GameState theState;
	return theState;
}

} // namespace

bool isSquadsGame()
{
	return state().playing && !config().types.empty();
}

void startGame()
{
	state().book.clear();
	state().playing = true;
}

void endGame()
{
	state().book.clear();
	state().playing = false;
	forgetOrders();
}

SquadBook &book()
{
	return state().book;
}

Object *memberObject(MemberId member)
{
	Object *object = TheGameLogic->findObjectByID((ObjectID)member);
	if (object == nullptr || object->isEffectivelyDead())
		return nullptr;
	return object;
}

std::vector<Object *> livingMembers(const Squad &squad)
{
	std::vector<Object *> objects;
	for (MemberId member : squad.members())
	{
		if (Object *object = memberObject(member))
			objects.push_back(object);
	}
	return objects;
}

Squad *squadOfObject(const Object *object)
{
	if (object == nullptr || !isSquadsGame())
		return nullptr;
	return book().find(book().squadOf((MemberId)object->getID()));
}

void onObjectDestroyed(Object *object)
{
	book().leave((MemberId)object->getID());
	forgetMember((MemberId)object->getID());
}

const SquadType *squadTypeOf(const ThingTemplate *unit)
{
	if (unit == nullptr || !isSquadsGame())
		return nullptr;
	return config().types.find(unit->getName().str());
}

} // namespace CoH::Squads
