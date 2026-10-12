// IBM Engine: the SquadsDemo flag, a ready-made test setup for squads. With it (and Squads) on,
// every human player of a skirmish or LAN game starts with, next to their Command Center:
//   - every faction's Barracks, so every squad has somewhere to heal and get members back,
//   - one of each squad, the transports of every faction and a Chinook, and $100,000,
// and the first enemy player gets a small group a little further towards the map's middle:
// squads to shoot at, vehicles for Kell's snipe and Black Lotus's hack, a helicopter for the
// anti-air squads, and a building to capture or blow up.
//
// It all happens at game start inside the game logic, so replays of demo games play back.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/BuildAssistant.h"
#include "Common/Money.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Team.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Module/CreateModule.h"
#include "GameLogic/PartitionManager.h"
#include "GameLogic/TerrainLogic.h"

#include "Features/FeatureFlags/FeatureFlags.h"
#include "Features/Squads/Squads.h"

namespace
{

const char *const s_friendlyBuildings[] =
{
	"AmericaBarracks", "ChinaBarracks", "GLABarracks",
};

const char *const s_friendlyUnits[] =
{
	"IbmUSARangerSquad", "IbmUSAMissileSquad", "IbmUSAAntiAirSquad", "IbmUSASealTeam",
	"IbmChinaRedguardSquad", "IbmChinaTankHunterSquad", "IbmChinaAntiAirSquad", "IbmChinaInfiltrators",
	"IbmGLARebelSquad", "IbmGLARPGSquad", "IbmGLAAntiAirSquad", "IbmGLASuicideSquad",
	"AmericaVehicleHumvee", "ChinaVehicleTroopCrawler", "GLAVehicleTechnical", "AmericaVehicleChinook",
};

const char *const s_enemyBuildings[] =
{
	"ChinaSupplyCenter",
};

const char *const s_enemyUnits[] =
{
	"IbmGLARebelSquad", "IbmChinaRedguardSquad", "IbmUSARangerSquad",
	"ChinaTankBattleMaster", "GLAVehicleTechnical", "AmericaVehicleComanche",
};

const Int DEMO_MONEY = 100000;
const Real FRIENDLY_DISTANCE = 220.0f;	///< from the Command Center towards the map's middle
const Real ENEMY_DISTANCE = 650.0f;

// Puts an object in the world as if the map had placed it (see GameLogic's placeObjectAtPosition).
Object *placeObject(const char *templateName, Player *player, const Coord3D &where)
{
	const ThingTemplate *tmpl = TheThingFactory->findTemplate(templateName);
	DEBUG_ASSERTCRASH(tmpl != nullptr, ("SquadsDemo: no object '%s'", templateName));
	if (tmpl == nullptr)
		return nullptr;

	Team *team = player->getDefaultTeam();
	Object *obj = TheThingFactory->newObject(tmpl, team);
	if (obj == nullptr)
		return nullptr;

	Coord3D pos = where;
	pos.z = TheTerrainLogic->getGroundHeight(pos.x, pos.y);
	obj->setOrientation(tmpl->getPlacementViewAngle());
	obj->setPosition(&pos);

	for (BehaviorModule **m = obj->getBehaviorModules(); *m; ++m)
	{
		CreateModuleInterface *create = (*m)->getCreate();
		if (create != nullptr)
			create->onBuildComplete();
	}

	if (team != nullptr)
		team->setActive();
	TheAI->pathfinder()->addObjectToPathfindMap(obj);

	if (obj->isKindOf(KINDOF_STRUCTURE))
	{
		player->onStructureCreated(nullptr, obj);
		player->onStructureConstructionComplete(nullptr, obj, FALSE);
	}
	else
	{
		player->onUnitCreated(nullptr, obj);
	}

	ThePartitionManager->update(); // so the next placement sees this one
	return obj;
}

// A spot near center where the building fits, searching outwards in rings.
Bool findBuildingSpot(const char *templateName, Player *player, const Coord3D &center, Coord3D &spot)
{
	const ThingTemplate *tmpl = TheThingFactory->findTemplate(templateName);
	if (tmpl == nullptr)
		return FALSE;

	const UnsignedInt options = BuildAssistant::TERRAIN_RESTRICTIONS | BuildAssistant::NO_OBJECT_OVERLAP;
	for (Real radius = 0.0f; radius <= 400.0f; radius += 40.0f)
	{
		const Int steps = radius > 0.0f ? 12 : 1;
		for (Int i = 0; i < steps; ++i)
		{
			const Real angle = (2.0f * PI * i) / steps;
			spot = center;
			spot.x += radius * Cos(angle);
			spot.y += radius * Sin(angle);
			spot.z = TheTerrainLogic->getGroundHeight(spot.x, spot.y);
			if (TheBuildAssistant->isLocationLegalToBuild(&spot, tmpl, tmpl->getPlacementViewAngle(), options, nullptr, player) == LBC_OK)
				return TRUE;
		}
	}
	return FALSE;
}

// A free spot for a unit near center.
Bool findUnitSpot(const Coord3D &center, Coord3D &spot)
{
	FindPositionOptions options;
	options.minRadius = 0.0f;
	options.maxRadius = 250.0f;
	options.startAngle = 0.0f;
	return ThePartitionManager->findPositionAround(&center, &options, &spot);
}

void placeGroup(Player *player, const Coord3D &center, const char *const *buildings, Int buildingCount, const char *const *units, Int unitCount)
{
	for (Int i = 0; i < buildingCount; ++i)
	{
		Coord3D spot;
		if (findBuildingSpot(buildings[i], player, center, spot))
			placeObject(buildings[i], player, spot);
		else
			DEBUG_LOG(("SquadsDemo: no room for %s", buildings[i]));
	}

	for (Int i = 0; i < unitCount; ++i)
	{
		Coord3D spot;
		if (findUnitSpot(center, spot))
			placeObject(units[i], player, spot);
		else
			DEBUG_LOG(("SquadsDemo: no room for %s", units[i]));
	}
}

struct CommandCenterSearch
{
	Object *found;
};

void findCommandCenter(Object *obj, void *userData)
{
	CommandCenterSearch *search = static_cast<CommandCenterSearch *>(userData);
	if (search->found == nullptr && obj->isKindOf(KINDOF_COMMANDCENTER) && !obj->isEffectivelyDead())
		search->found = obj;
}

// The first player at war with this one, preferring the computer.
Player *findEnemy(Player *player)
{
	Player *fallback = nullptr;
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player *other = ThePlayerList->getNthPlayer(i);
		if (other == player || !other->isPlayableSide() || other->getDefaultTeam() == nullptr)
			continue;
		if (player->getRelationship(other->getDefaultTeam()) != ENEMIES)
			continue;
		if (other->getPlayerType() == PLAYER_COMPUTER)
			return other;
		if (fallback == nullptr)
			fallback = other;
	}
	return fallback;
}

} // namespace

//-------------------------------------------------------------------------------------------------
void IbmEngine::setUpSquadsDemo()
{
	if (!isFeatureActive(Feature_SquadsDemo))
		return;

	if (!isFeatureActive(Feature_Squads))
	{
		DEBUG_LOG(("SquadsDemo: needs the Squads feature; nothing placed"));
		return;
	}

	Region3D extent;
	TheTerrainLogic->getExtent(&extent);
	Coord3D mapCenter;
	mapCenter.x = (extent.lo.x + extent.hi.x) / 2.0f;
	mapCenter.y = (extent.lo.y + extent.hi.y) / 2.0f;
	mapCenter.z = 0.0f;

	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		if (player->getPlayerType() != PLAYER_HUMAN || !player->isPlayableSide())
			continue;

		CommandCenterSearch search;
		search.found = nullptr;
		player->iterateObjects(findCommandCenter, &search);
		if (search.found == nullptr)
			continue;

		// Towards the map's middle, where there is usually open ground.
		const Coord3D base = *search.found->getPosition();
		Coord2D dir;
		dir.x = mapCenter.x - base.x;
		dir.y = mapCenter.y - base.y;
		if (dir.length() < 1.0f)
			dir.x = 1.0f;
		dir.normalize();

		player->getMoney()->deposit(DEMO_MONEY, FALSE, FALSE);

		Coord3D friendly = base;
		friendly.x += dir.x * FRIENDLY_DISTANCE;
		friendly.y += dir.y * FRIENDLY_DISTANCE;
		placeGroup(player, friendly, s_friendlyBuildings, ARRAY_SIZE(s_friendlyBuildings), s_friendlyUnits, ARRAY_SIZE(s_friendlyUnits));

		Player *enemy = findEnemy(player);
		if (enemy != nullptr)
		{
			Coord3D hostile = base;
			hostile.x += dir.x * ENEMY_DISTANCE;
			hostile.y += dir.y * ENEMY_DISTANCE;
			placeGroup(enemy, hostile, s_enemyBuildings, ARRAY_SIZE(s_enemyBuildings), s_enemyUnits, ARRAY_SIZE(s_enemyUnits));
		}

		DEBUG_LOG(("SquadsDemo: set up for player %d", i));
	}
}
