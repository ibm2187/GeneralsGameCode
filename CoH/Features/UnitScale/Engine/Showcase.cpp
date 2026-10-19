// CoH UnitScale: the -cohUnitScaleShowcase test option. At the start of a new game it lines up each
// faction's soldiers beside its vehicles, in front of the local player's command center, and points
// the camera at them, so the proportions can be judged and compared in screenshots. It needs the flag
// on; for the retail sizes, install the pack of make_pack.py --preset retail, which scales nothing.
// Never used in a real game: it changes what is on the map.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameClient/View.h"
#include "GameLogic/AI.h"
#include "GameLogic/AIPathfind.h"
#include "GameLogic/Object.h"
#include "GameLogic/TerrainLogic.h"

#include "Kernel/Kernel.h"
#include "Kernel/Log.h"
#include "Kernel/Objects.h"

#include "Features/UnitScale/Engine/Internal.h"

#include <array>

namespace CoH::UnitScale
{

namespace
{

// One row per faction, from the lightest vehicle to the biggest, a soldier standing beside each.
struct Row
{
	const char *soldier;
	std::array<const char *, 6> vehicles;
};

constexpr std::array<Row, 3> rows { {
	{ "AmericaInfantryRanger", { "AmericaVehicleHumvee", "AmericaTankCrusader", "AmericaTankPaladin",
		"AmericaVehicleTomahawk", "AmericaVehicleDozer", "AmericaVehicleComanche" } },
	{ "ChinaInfantryRedguard", { "ChinaVehicleTroopCrawler", "ChinaTankBattleMaster", "ChinaTankGattling",
		"ChinaVehicleInfernoCannon", "ChinaTankOverlord", "ChinaVehicleHelix" } },
	{ "GLAInfantryRebel", { "GLAVehicleTechnicalChassisOne", "GLAVehicleRocketBuggy", "GLATankScorpion",
		"GLATankMarauder", "GLAVehicleScudLauncher", "GLAVehicleBattleBus" } },
} };

constexpr Real vehicleGap = 70.0f;	///< between vehicles, room for the biggest scaled hull
constexpr Real soldierBeside = 30.0f;	///< from a vehicle's centre to the soldier at its side
constexpr Real rowGap = 90.0f;
constexpr Real distanceFromBase = 160.0f;

Coord3D onGround(Real x, Real y)
{
	Coord3D position;
	position.set(x, y, TheTerrainLogic->getGroundHeight(x, y));
	return position;
}

void spawn(const char *name, Team *team, const Coord3D &position, Real angle)
{
	const ThingTemplate *kind = TheThingFactory->findTemplate(name);
	if (kind == nullptr)
	{
		log("UnitScale showcase: no object %s", name);
		return;
	}
	Object *object = TheThingFactory->newObject(kind, team);
	object->setPosition(&position);
	object->setOrientation(angle);
	TheAI->pathfinder()->addObjectToPathfindMap(object);
}

// Along +x from 'start', facing the camera's usual view (units pointing -y).
void spawnRow(const Row &row, Team *team, const Coord3D &start)
{
	Real x = start.x;
	for (const char *vehicle : row.vehicles)
	{
		x += vehicleGap;
		spawn(vehicle, team, onGround(x, start.y), -PI / 2.0f);
		spawn(row.soldier, team, onGround(x - soldierBeside, start.y), -PI / 2.0f);
	}
}

Object *commandCenterOf(const Player &player)
{
	for (Object *building : objectsOf(player, { .anyKind = kinds(KINDOF_COMMANDCENTER) }))
		return building;
	return nullptr;
}

void onGameStartAfterMap(bool loadingSave)
{
	static const bool enabled = hasCommandLineOption("-cohUnitScaleShowcase");
	Player *player = ThePlayerList->getLocalPlayer();
	if (!enabled || loadingSave || player == nullptr)
		return;
	const Object *base = commandCenterOf(*player);
	if (base == nullptr)
		return;

	// Toward the middle of the map, so the rows stand on open ground.
	Region3D extent;
	TheTerrainLogic->getExtent(&extent);
	const Coord3D &at = *base->getPosition();
	const Real towardMiddle = (at.y < (extent.lo.y + extent.hi.y) / 2.0f) ? 1.0f : -1.0f;
	const Real firstRowY = at.y + towardMiddle * distanceFromBase;
	const Real startX = at.x - 3.5f * vehicleGap;

	Team *team = player->getDefaultTeam();
	Real y = firstRowY;
	for (const Row &row : rows)
	{
		spawnRow(row, team, onGround(startX, y));
		y += towardMiddle * rowGap;
	}

	if (TheTacticalView != nullptr)
	{
		const Coord3D middle = onGround(at.x, firstRowY + towardMiddle * rowGap);
		TheTacticalView->lookAt(&middle);
	}
	log("UnitScale showcase: three rows in front of %s", base->getTemplate()->getName().str());
}

} // namespace

void initShowcase(Feature &self)
{
	Hooks::gameStartAfterMap().subscribe(self, onGameStartAfterMap);
}

} // namespace CoH::UnitScale
