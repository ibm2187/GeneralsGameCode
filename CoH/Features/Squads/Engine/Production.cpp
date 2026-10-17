// CoH Squads: an order for a squad unit makes the whole squad. The factory makes the ordered unit as
// always; the rest of the squad is made with it and walks out of the same door to the same rally
// point. The order costs, and takes, what the squad's members would, scaled by the pack.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Module/CreateModule.h"
#include "GameLogic/Module/ProductionUpdate.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/UpdateModule.h"
#include "GameLogic/Object.h"

#include "Kernel/Log.h"

#include "Features/Squads/Engine/Config.h"
#include "Features/Squads/Engine/Internal.h"

#include <map>
#include <utility>
#include <vector>

namespace CoH::Squads
{

namespace
{

// The squad each factory is making, while an order that makes several units (Red Guards) still has
// units to come: they join it rather than start squads of their own.
std::map<ObjectID, SquadId> &openSquads()
{
	static std::map<ObjectID, SquadId> theSquads;
	return theSquads;
}

// Units a factory makes several of per order (Red Guards: 2), from every factory's QuantityModifier
// lines. Template data only, so every machine finds the same; read once per game.
std::vector<std::pair<const ThingTemplate *, int>> &orderQuantities()
{
	static std::vector<std::pair<const ThingTemplate *, int>> theQuantities;
	return theQuantities;
}

void readOrderQuantities()
{
	orderQuantities().clear();
	for (const ThingTemplate *factory = TheThingFactory->firstTemplate(); factory != nullptr;
		factory = factory->friend_getNextTemplate())
	{
		const ModuleInfo &modules = factory->getBehaviorModuleInfo();
		for (Int i = 0; i < modules.getCount(); ++i)
		{
			if (modules.getNthName(i) != "ProductionUpdate")
				continue;
			const ProductionUpdateModuleData *data = (const ProductionUpdateModuleData *)modules.getNthData(i);
			for (const QuantityModifier &modifier : data->m_quantityModifiers)
			{
				const ThingTemplate *made = TheThingFactory->findTemplate(modifier.m_templateName);
				if (made != nullptr && modifier.m_quantity > 1)
					orderQuantities().push_back({ made, modifier.m_quantity });
			}
		}
	}
}

// How many units one order of 'unit' makes.
int orderQuantity(const ThingTemplate *unit)
{
	for (const auto &entry : orderQuantities())
	{
		if (entry.first->isEquivalentTo(unit))
			return entry.second;
	}
	return 1;
}

// The escorts' build cost added together.
int escortCost(const SquadType &type)
{
	int cost = 0;
	for (const Escort &escort : type.escorts)
	{
		if (const ThingTemplate *unit = TheThingFactory->findTemplate(escort.unit.c_str()))
			cost += unit->friend_getBuildCost() * escort.count;
	}
	return cost;
}

float maxHealthOf(const Object *object)
{
	const BodyModuleInterface *body = object->getBodyModule();
	return body != nullptr ? body->getMaxHealth() : 0.0f;
}

void finishBuilding(Object *member)
{
	// What ProductionUpdate does for the unit it makes: the create modules' part for a built unit.
	for (BehaviorModule **module = member->getBehaviorModules(); *module; ++module)
	{
		if (CreateModuleInterface *create = (*module)->getCreate())
			create->onBuildComplete();
	}
}

// Makes one more member of the squad, at the factory, on its way to the same place as the unit.
void makeMember(Squad &squad, const ThingTemplate *kind, Object *factory, Object *unit)
{
	Object *member = TheThingFactory->newObject(kind, unit->getTeam());
	if (member == nullptr)
		return;
	member->setProducer(factory);

	ExitInterface *exit = factory->getObjectExitInterface();
	if (exit != nullptr)
		exit->exitObjectViaDoor(member, DOOR_1);
	else
		member->setPosition(unit->getPosition());

	finishBuilding(member);
	book().join(squad.id(), (MemberId)member->getID(), maxHealthOf(member));
}

int firstUnitsToCome(Object *factory)
{
	const ProductionUpdateInterface *queue = factory->getProductionUpdateInterface();
	const ProductionEntry *entry = queue != nullptr ? queue->firstProduction() : nullptr;
	if (entry == nullptr)
		return 0;
	// This hook comes before the entry counts the unit just made.
	return entry->getProductionQuantityRemaining() - 1;
}

bool joinsOpenSquad(Object *factory, Object *unit)
{
	const auto open = openSquads().find(factory->getID());
	if (open == openSquads().end())
		return false;
	const bool joined = book().join(open->second, (MemberId)unit->getID(), maxHealthOf(unit));
	if (firstUnitsToCome(factory) <= 0)
		openSquads().erase(open);
	return joined;
}

} // namespace

void onUnitProduced(Object *factory, Object *unit)
{
	if (factory == nullptr || unit == nullptr)
		return;
	const SquadType *type = squadTypeOf(unit->getTemplate());
	if (type == nullptr)
		return;
	if (joinsOpenSquad(factory, unit))
		return;

	const int quantity = orderQuantity(unit->getTemplate());
	Squad &squad = book().create(unit->getTemplate()->getName().str(), type->memberCount());
	book().join(squad.id(), (MemberId)unit->getID(), maxHealthOf(unit));

	for (int i = quantity; i < type->size; ++i)
		makeMember(squad, unit->getTemplate(), factory, unit);
	for (const Escort &escort : type->escorts)
	{
		const ThingTemplate *kind = TheThingFactory->findTemplate(escort.unit.c_str());
		for (int i = 0; kind != nullptr && i < escort.count; ++i)
			makeMember(squad, kind, factory, unit);
	}

	if (firstUnitsToCome(factory) > 0)
		openSquads()[factory->getID()] = squad.id();

	CoH::log("Squads: frame %u player %d made squad %d of %d %s", TheGameLogic->getFrame(),
		unit->getControllingPlayer()->getPlayerIndex(), squad.id(), squad.memberCount(), squad.unit().c_str());
}

void adjustBuildCost(const Player * /*player*/, const ThingTemplate *unit, int &cost)
{
	const SquadType *type = squadTypeOf(unit);
	if (type == nullptr)
		return;
	cost = squadAmount(cost, orderQuantity(unit), type->size, escortCost(*type), config().costPercent);
}

void adjustBuildTime(const Player * /*player*/, const ThingTemplate *unit, int &frames)
{
	const SquadType *type = squadTypeOf(unit);
	if (type == nullptr)
		return;
	// Escorts take as long per dollar as the ordered unit does.
	const int escortFrames = (int)((long long)escortCost(*type) * frames / max(1, unit->friend_getBuildCost()));
	frames = squadAmount(frames, orderQuantity(unit), type->size, escortFrames, config().timePercent);
}

int rawSquadCost(const ThingTemplate *unit)
{
	if (unit == nullptr)
		return 0;
	const int cost = unit->friend_getBuildCost();
	const SquadType *type = squadTypeOf(unit);
	if (type == nullptr)
		return cost;
	return squadAmount(cost, orderQuantity(unit), type->size, escortCost(*type), config().costPercent);
}

void startProduction()
{
	openSquads().clear();
	readOrderQuantities();
}

void forgetProduction()
{
	openSquads().clear();
	orderQuantities().clear();
}

} // namespace CoH::Squads
