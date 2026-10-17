// CoH Sectors: a new game's sectors. Every supply source and tech building on the map becomes a point
// (Domain/Points), each made into a CoHSectorPoint object. The supplies themselves are emptied, so
// nothing can be gathered, and the AI's build lists lose the supply buildings.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Team.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/SupplyWarehouseDockUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/SidesList.h"
#include "GameLogic/TerrainLogic.h"
#include "GameNetwork/GameInfo.h"

#include "Kernel/Log.h"

#include "Features/Sectors/Engine/Config.h"
#include "Features/Sectors/Engine/Internal.h"
#include "Features/Sectors/Engine/SectorPointUpdate.h"
#include "Features/Sectors/Engine/World.h"

#include <algorithm>
#include <cmath>

namespace CoH::Sectors
{

namespace
{

bool isPlayingPlayer(const Player *player)
{
	return player && player->isPlayableSide() && player->isPlayerActive();
}

// Each playing player's start: where its first command center stands.
std::vector<Start> findStarts()
{
	std::vector<Start> starts;
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		const Player *player = ThePlayerList->getNthPlayer(i);
		if (!isPlayingPlayer(player))
			continue;

		for (Object *object = TheGameLogic->getFirstObject(); object; object = object->getNextObject())
		{
			if (object->isKindOf(KINDOF_COMMANDCENTER) && object->getControllingPlayer() == player)
			{
				starts.push_back({ toVec2(*object->getPosition()), i });
				break;
			}
		}
	}
	return starts;
}

// The map's supply sources and tech buildings, which belong to no playing player.
std::vector<Source> findSources()
{
	std::vector<Source> sources;
	for (Object *object = TheGameLogic->getFirstObject(); object; object = object->getNextObject())
	{
		if (isPlayingPlayer(object->getControllingPlayer()) || object->isEffectivelyDead())
			continue;

		if (object->isAnyKindOf(config().highValueKinds))
			sources.push_back({ toVec2(*object->getPosition()), SectorKind::HighValue, (int)object->getID() });
		else if (object->isKindOf(KINDOF_SUPPLY_SOURCE))
			sources.push_back({ toVec2(*object->getPosition()), SectorKind::Supply, (int)object->getID() });
	}
	return sources;
}

Bool makePoint(const ThingTemplate *pointTemplate, const PointPlan &plan)
{
	Object *object = TheThingFactory->newObject(pointTemplate, ThePlayerList->getNeutralPlayer()->getDefaultTeam());
	SectorPointUpdate *point = SectorPointUpdate::find(object);
	if (point == nullptr)
	{
		CoH::log("Sectors: %s has no SectorPointUpdate; no sectors this game", pointTemplate->getName().str());
		if (object)
			TheGameLogic->destroyObject(object);
		return FALSE;
	}

	Coord3D position = { plan.position.x, plan.position.y, 0.0f };
	position.z = TheTerrainLogic->getGroundHeight(position.x, position.y);
	object->setPosition(&position);

	std::vector<ObjectID> members;
	for (int id : plan.members)
		members.push_back((ObjectID)id);
	point->setUp(plan.kind, plan.homePlayer, members);
	return TRUE;
}

} // namespace

void emptySupplies(Object *object)
{
	static const NameKeyType key = NAMEKEY("SupplyWarehouseDockUpdate");
	SupplyWarehouseDockUpdate *warehouse = object ? (SupplyWarehouseDockUpdate *)object->findUpdateModule(key) : nullptr;
	if (warehouse && warehouse->getBoxesStored() > 0)
		warehouse->setCashValue(0);
}

// A replay plays in its own game mode; one of a skirmish or network game has that game's options.
Bool isSectorGame()
{
	if (TheGameLogic->isInSkirmishGame() || TheGameLogic->isInMultiplayerGame())
		return TRUE;
	return TheGameLogic->isInReplayGame() && TheGameInfo != nullptr;
}

void setUpNewGame()
{
	if (!isSectorGame())
	{
		CoH::log("Sectors: not a skirmish or multiplayer game; no sectors");
		return;
	}

	const ThingTemplate *pointTemplate = TheThingFactory->findTemplate(config().pointObject);
	if (pointTemplate == nullptr)
	{
		CoH::log("Sectors: no '%s' object in the pack; no sectors", config().pointObject.str());
		return;
	}

	const std::vector<Start> starts = findStarts();
	const std::vector<PointPlan> plans = planPoints(findSources(), starts, config().points);
	for (const PointPlan &plan : plans)
	{
		if (!makePoint(pointTemplate, plan))
			return;
		CoH::log("Sectors: point %s at (%.0f, %.0f) home %d from %d buildings",
			plan.kind == SectorKind::Home ? "home" : plan.kind == SectorKind::HighValue ? "high-value" : "supply",
			plan.position.x, plan.position.y, plan.homePlayer, (int)plan.members.size());
	}

	for (Object *object = TheGameLogic->getFirstObject(); object; object = object->getNextObject())
		emptySupplies(object);

	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		if (player->getPlayerType() == PLAYER_COMPUTER && isPlayingPlayer(player))
		{
			adaptAIBuildList(player);
			logAIBuildList(player, starts);
		}
	}
}

} // namespace CoH::Sectors
