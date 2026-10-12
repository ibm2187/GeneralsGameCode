// IBM Engine: squads get their lost members back while they heal at their Barracks. Every
// ReinforceDelay, a squad within ReinforceRadius of one of its owner's ReinforceAt buildings gets
// back its first lost member, in SpawnTemplateName order, for free. The soldier comes out of the
// building's door and the mob code walks him to his squad.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/Module/ContainModule.h"
#include "GameLogic/PartitionManager.h"

#include "Features/Squads/SquadUpdate.h"

namespace
{

struct BuildingSearch
{
	const IbmSquadUpdateModuleData *data;
	Coord3D pos;
	Object *best;
	Real bestDistSqr;
};

void findClosestBuilding(Object *obj, void *userData)
{
	BuildingSearch *search = static_cast<BuildingSearch *>(userData);

	if (obj->isEffectivelyDead() || obj->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION) || obj->testStatus(OBJECT_STATUS_SOLD))
		return;

	// Being unpowered doesn't matter; EMP, hacks and the like do.
	if (obj->isDisabled() && !(obj->getDisabledFlags().count() == 1 && obj->isDisabledByType(DISABLED_UNDERPOWERED)))
		return;

	const std::vector<AsciiString> &names = search->data->m_reinforceAt;
	if (std::find(names.begin(), names.end(), obj->getTemplate()->getName()) == names.end())
		return;

	if (obj->getObjectExitInterface() == nullptr)
		return;

	const Real distSqr = ThePartitionManager->getDistanceSquared(obj, &search->pos, FROM_BOUNDINGSPHERE_2D);
	if (distSqr <= sqr(search->data->m_reinforceRadius) && distSqr < search->bestDistSqr)
	{
		search->best = obj;
		search->bestDistSqr = distSqr;
	}
}

// The first lost member in spawn order, or nullptr if the squad is complete.
const ThingTemplate *findLostMember(const Object *leader)
{
	const SpawnBehaviorModuleData *spawnData = IbmEngine::findSquadSpawnData(leader->getTemplate());
	if (spawnData == nullptr || spawnData->m_spawnTemplateNameData.empty())
		return nullptr;

	// The full squad. SpawnBehavior cycles through the names until it has SpawnNumber.
	const std::vector<AsciiString> &names = spawnData->m_spawnTemplateNameData;
	std::vector<AsciiString> missing;
	for (Int i = 0; i < spawnData->m_spawnNumberData; ++i)
		missing.push_back(names[i % names.size()]);

	IbmEngine::forEachSquadMember(leader, [&](Object *member)
	{
		std::vector<AsciiString>::iterator it = std::find(missing.begin(), missing.end(), member->getTemplate()->getName());
		if (it != missing.end())
			missing.erase(it);
	});

	return missing.empty() ? nullptr : TheThingFactory->findTemplate(missing.front());
}

} // namespace

//-------------------------------------------------------------------------------------------------
Bool IbmEngine::reinforceSquad(Object *leader, const IbmSquadUpdateModuleData *data)
{
	Player *player = leader->getControllingPlayer();
	if (player == nullptr || data->m_reinforceAt.empty())
		return FALSE;

	const ThingTemplate *soldierTemplate = findLostMember(leader);
	if (soldierTemplate == nullptr)
		return FALSE;

	BuildingSearch search;
	search.data = data;
	search.pos = *leader->getPosition();
	search.best = nullptr;
	search.bestDistSqr = sqr(data->m_reinforceRadius) + 1.0f;
	player->iterateObjects(findClosestBuilding, &search);
	if (search.best == nullptr)
		return FALSE;

	Object *soldier = TheThingFactory->newObject(soldierTemplate, leader->getTeam());
	player->onUnitCreated(leader, soldier); // counts as a unit made, like the squad's first members

	// Out of the building's door, like a soldier it trained.
	ExitInterface *exit = search.best->getObjectExitInterface();
	ExitDoorType door = exit->reserveDoorForExit(soldierTemplate, soldier);
	if (door != DOOR_NONE_AVAILABLE)
	{
		exit->exitObjectViaDoor(soldier, door);
	}
	else
	{
		Coord3D pos;
		if (!exit->getExitPosition(pos))
			pos = *search.best->getPosition();
		soldier->setPosition(&pos);
	}

	// After the exit, which would make the building his producer.
	leader->getSpawnBehaviorInterface()->adoptSlave(soldier);
	return TRUE;
}
