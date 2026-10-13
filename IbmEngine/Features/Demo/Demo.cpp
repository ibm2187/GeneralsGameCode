// IBM Engine: the Demo flag, a ready-made test setup for the features that are on. Every human
// player of a skirmish or LAN game starts with $100,000 and, toward the map's middle from their
// Command Center:
//   - Squads: every faction's Barracks, one of each squad, the transports of every faction and a
//     Chinook; the first enemy player gets a small group further on: squads to shoot at, vehicles
//     for Kell's snipe and Black Lotus's hack, a helicopter for the anti-air squads, and a building
//     to capture or blow up.
//   - Economy: every faction's War Factory, Airfield and tech buildings (with power plants and the
//     sciences the tanks need), so every tank can be built and timed, plus one of each tank and
//     helicopter; the first enemy player gets a tank group to fight.
//   - AirPower: the top general rank, with its promotion points to spend and the rank-5 super weapon
//     strike, and the new promotions (Precision Strike, Napalm Strike, Mine Rockets) already granted.
// Accuracy and Armor need nothing of their own: the enemy groups are targets.
//
// It all happens at game start inside the game logic, so replays of demo games play back. The
// squads part comes first and is unchanged from the old SquadsDemo flag, so its replays still match.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/BuildAssistant.h"
#include "Common/Money.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Science.h"
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
#include "Features/Demo/Demo.h"

namespace
{

// Squads
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

// Economy
const char *const s_economyBuildings[] =
{
	"AmericaPowerPlant", "AmericaPowerPlant", "ChinaPowerPlant", "ChinaPowerPlant",
	"AmericaWarFactory", "AmericaAirfield", "AmericaStrategyCenter",
	"ChinaWarFactory", "ChinaAirfield", "ChinaPropagandaCenter",
	"GLAArmsDealer", "GLAPalace",
};

const char *const s_economySciences[] =
{
	"SCIENCE_PaladinTank", "SCIENCE_NukeLauncher", "SCIENCE_ScudLauncher", "SCIENCE_MarauderTank",
};

const char *const s_economyUnits[] =
{
	"AmericaVehicleHumvee", "AmericaTankCrusader", "AmericaTankPaladin", "AmericaTankMicrowave",
	"AmericaTankAvenger", "AmericaVehicleTomahawk", "AmericaVehicleMedic", "AmericaVehicleComanche",
	"ChinaTankGattling", "ChinaTankBattleMaster", "ChinaTankOverlord", "ChinaTankDragon", "ChinaTankECM",
	"ChinaVehicleInfernoCannon", "ChinaVehicleNukeLauncher", "ChinaVehicleTroopCrawler",
	"ChinaVehicleListeningOutpost", "ChinaVehicleHelix",
	"GLAVehicleTechnical", "GLAVehicleRocketBuggy", "GLAVehicleCombatBike", "GLATankScorpion",
	"GLATankMarauder", "GLAVehicleQuadCannon", "GLAVehicleToxinTruck", "GLAVehicleBombTruck",
	"GLAVehicleScudLauncher", "GLAVehicleBattleBus", "GLAVehicleRadarVan",
	"AmericaVehicleDozer", "ChinaVehicleSupplyTruck",	// exempt: retail stats, to compare
};

const char *const s_economyEnemyUnits[] =
{
	"AmericaTankCrusader", "AmericaTankCrusader", "ChinaTankBattleMaster", "ChinaTankBattleMaster",
	"ChinaTankOverlord", "GLATankScorpion", "GLATankScorpion", "GLATankMarauder", "ChinaVehicleHelix",
};

// AirPower: the new promotions, granted outright (each side's Command Center only has its own).
const char *const s_airPowerSciences[] =
{
	"SCIENCE_IbmPrecisionStrike", "SCIENCE_NapalmStrike", "SCIENCE_IbmMineRockets",
};

const Int DEMO_MONEY = 100000;
const Real FRIENDLY_DISTANCE = 220.0f;	///< from the Command Center towards the map's middle
const Real ENEMY_DISTANCE = 650.0f;
const Real ECONOMY_SIDE_DISTANCE = 450.0f;	///< the Economy groups sit beside the Squads ones, across the line to the middle

// Puts an object in the world as if the map had placed it (see GameLogic's placeObjectAtPosition).
Object *placeObject(const char *templateName, Player *player, const Coord3D &where)
{
	const ThingTemplate *tmpl = TheThingFactory->findTemplate(templateName);
	DEBUG_ASSERTCRASH(tmpl != nullptr, ("Demo: no object '%s'", templateName));
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
			DEBUG_LOG(("Demo: no room for %s", buildings[i]));
	}

	for (Int i = 0; i < unitCount; ++i)
	{
		Coord3D spot;
		if (findUnitSpot(center, spot))
			placeObject(units[i], player, spot);
		else
			DEBUG_LOG(("Demo: no room for %s", units[i]));
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

void clampInto(Coord3D &pos, const Region3D &region)
{
	pos.x = clamp(region.lo.x, pos.x, region.hi.x);
	pos.y = clamp(region.lo.y, pos.y, region.hi.y);
}

void grantSciences(Player *player, const char *const *sciences, Int count)
{
	for (Int i = 0; i < count; ++i)
	{
		const ScienceType science = TheScienceStore->getScienceFromInternalName(sciences[i]);
		if (science != SCIENCE_INVALID)
			player->grantScience(science);
		else
			DEBUG_LOG(("Demo: no science %s", sciences[i]));
	}
}

} // namespace

//-------------------------------------------------------------------------------------------------
void IbmEngine::setUpDemo()
{
	if (!isFeatureActive(Feature_Demo))
		return;

	const Bool squads = isFeatureActive(Feature_Squads);
	const Bool economy = isFeatureActive(Feature_Economy);
	const Bool airPower = isFeatureActive(Feature_AirPower);
	if (!squads && !economy && !airPower)
	{
		DEBUG_LOG(("Demo: no feature with a demo is on (Squads, Economy, AirPower); nothing placed"));
		return;
	}

	Region3D extent;
	TheTerrainLogic->getExtent(&extent);
	Coord3D mapCenter;
	mapCenter.x = (extent.lo.x + extent.hi.x) / 2.0f;
	mapCenter.y = (extent.lo.y + extent.hi.y) / 2.0f;
	mapCenter.z = 0.0f;

	// Group centers stay this far inside the map's edge.
	const Real margin = 150.0f;
	Region3D inner = extent;
	inner.lo.x += margin;
	inner.lo.y += margin;
	inner.hi.x -= margin;
	inner.hi.y -= margin;

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
		Player *enemy = findEnemy(player);

		if (squads)
		{
			Coord3D friendly = base;
			friendly.x += dir.x * FRIENDLY_DISTANCE;
			friendly.y += dir.y * FRIENDLY_DISTANCE;
			placeGroup(player, friendly, s_friendlyBuildings, ARRAY_SIZE(s_friendlyBuildings), s_friendlyUnits, ARRAY_SIZE(s_friendlyUnits));

			if (enemy != nullptr)
			{
				Coord3D hostile = base;
				hostile.x += dir.x * ENEMY_DISTANCE;
				hostile.y += dir.y * ENEMY_DISTANCE;
				placeGroup(enemy, hostile, s_enemyBuildings, ARRAY_SIZE(s_enemyBuildings), s_enemyUnits, ARRAY_SIZE(s_enemyUnits));
			}
		}

		if (economy)
		{
			// Beside the line to the middle, so it doesn't crowd the Squads groups.
			Coord2D side;
			side.x = -dir.y;
			side.y = dir.x;

			grantSciences(player, s_economySciences, ARRAY_SIZE(s_economySciences));

			Coord3D friendly = base;
			friendly.x += dir.x * FRIENDLY_DISTANCE + side.x * ECONOMY_SIDE_DISTANCE;
			friendly.y += dir.y * FRIENDLY_DISTANCE + side.y * ECONOMY_SIDE_DISTANCE;
			clampInto(friendly, inner);
			placeGroup(player, friendly, s_economyBuildings, ARRAY_SIZE(s_economyBuildings), s_economyUnits, ARRAY_SIZE(s_economyUnits));

			if (enemy != nullptr)
			{
				Coord3D hostile = base;
				hostile.x += dir.x * ENEMY_DISTANCE + side.x * ECONOMY_SIDE_DISTANCE;
				hostile.y += dir.y * ENEMY_DISTANCE + side.y * ECONOMY_SIDE_DISTANCE;
				clampInto(hostile, inner);
				placeGroup(enemy, hostile, nullptr, 0, s_economyEnemyUnits, ARRAY_SIZE(s_economyEnemyUnits));
			}
		}

		if (airPower)
		{
			player->setRankLevel(5); // capped at the game's rank limit
			grantSciences(player, s_airPowerSciences, ARRAY_SIZE(s_airPowerSciences));
		}

		DEBUG_LOG(("Demo: set up for player %d", i));
	}
}
