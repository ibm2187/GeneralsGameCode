// CoH Sectors: the skirmish AI plays for sectors. Its scripts know nothing about them, and without
// sectors it earns nothing, so every few seconds each AI player sends idle combat units to the best
// targets of Domain/AiTargets: its contested sectors first, then the neighbors of its territory.
//
// It keeps no state of its own, so a loaded game plays on the same way: a unit counts toward a target
// while it stands on that point and the point still needs it, or while it is on its way there.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Money.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "GameLogic/AI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/TerrainLogic.h"
#include "GameLogic/Weapon.h"

#include "Kernel/Log.h"

#include "Features/Sectors/Domain/AiTargets.h"
#include "Features/Sectors/Engine/Config.h"
#include "Features/Sectors/Engine/Internal.h"
#include "Features/Sectors/Engine/SectorPointUpdate.h"
#include "Features/Sectors/Engine/World.h"

#include <algorithm>
#include <string>

namespace CoH::Sectors
{

namespace
{

bool isThreatened(const World &w, int index)
{
	const Occupation &occupation = w.occupation(index);
	const PlayerId owner = w.ownerOf(index);
	if (occupation.kind == Occupation::Kind::Contested)
		return true;
	return occupation.kind == Occupation::Kind::Held && owner != NoPlayer && areEnemies(occupation.side, owner);
}

std::vector<AiSector> viewFor(PlayerId player, const World &w)
{
	std::vector<AiSector> view;
	for (int i = 0; i < w.sectorCount(); ++i)
	{
		const PlayerId owner = w.ownerOf(i);
		AiSector sector;
		sector.position = w.sector(i).position;
		sector.kind = w.sector(i).kind;
		sector.owner = owner;
		sector.ownerIsFriend = owner != NoPlayer && owner != player && !areEnemies(owner, player);
		sector.locked = w.isLocked(i) && areEnemies(owner, player);
		sector.threatened = isThreatened(w, i);
		view.push_back(sector);
	}
	return view;
}

Vec2 baseOf(PlayerId player, const World &w)
{
	for (int i = 0; i < w.sectorCount(); ++i)
	{
		if (w.sector(i).kind == SectorKind::Home && w.sector(i).homePlayer == player)
			return w.sector(i).position;
	}
	return {};
}

bool isNear(const Coord3D &position, const Vec2 &point)
{
	const Real radius = config().captureRadius;
	return distanceSquared(toVec2(position), point) <= radius * radius;
}

// A point still needs the units on it until it is the player's, fully held and safe.
bool needsHolding(PlayerId player, const World &w, int index)
{
	const SectorPointUpdate *point = w.point(index);
	if (point == nullptr)
		return false;
	return point->state().owner != player || point->state().progress < 1.0f || isThreatened(w, index);
}

struct Units
{
	std::vector<int> committed;	///< per target: units on it or on their way
	std::vector<Object *> idle;	///< free to send
	int capturers = 0;			///< all the player's units that can capture
	int structures = 0;			///< for the log
};

Units findUnits(Player *player, const World &w, const std::vector<int> &targets)
{
	Units units;
	units.committed.assign(targets.size(), 0);
	const PlayerId me = player->getPlayerIndex();

	for (Object *object = TheGameLogic->getFirstObject(); object; object = object->getNextObject())
	{
		if (object->getControllingPlayer() == player && object->isKindOf(KINDOF_STRUCTURE))
			++units.structures;
		if (object->getControllingPlayer() != player || !isCapturer(object) || object->getAI() == nullptr)
			continue;
		++units.capturers;

		const AIUpdateInterface *ai = object->getAI();
		const bool idle = ai->isIdle();
		const Coord3D *goal = idle ? object->getPosition() : ai->getGoalPosition();
		bool counted = false;
		for (size_t t = 0; t < targets.size() && goal; ++t)
		{
			if (isNear(*goal, w.sector(targets[t]).position) && (!idle || needsHolding(me, w, targets[t])))
			{
				++units.committed[t];
				counted = true;
				break;
			}
		}

		// An idle unit holding a point that isn't a target right now stays there too.
		if (!counted && idle)
		{
			const int here = w.sectorAt(*object->getPosition());
			const bool holding = here >= 0 && isNear(*object->getPosition(), w.sector(here).position) && needsHolding(me, w, here);
			if (!holding)
				units.idle.push_back(object);
		}
	}
	return units;
}

void sendNearest(std::vector<Object *> &idle, const Vec2 &target, int count)
{
	Coord3D destination = { target.x, target.y, 0.0f };
	destination.z = TheTerrainLogic->getGroundHeight(destination.x, destination.y);

	for (int sent = 0; sent < count && !idle.empty(); ++sent)
	{
		auto nearest = std::min_element(idle.begin(), idle.end(), [&target](const Object *a, const Object *b)
		{
			return distanceSquared(toVec2(*a->getPosition()), target) < distanceSquared(toVec2(*b->getPosition()), target);
		});
		(*nearest)->getAI()->aiAttackMoveToPosition(&destination, NO_MAX_SHOTS_LIMIT, CMD_FROM_AI);
		idle.erase(nearest);
	}
}

// Once a minute per player, with -cohLog.
void logThinking(const Player *player, const std::vector<int> &targets, const Units &units)
{
	if (!CoH::isLogging() || TheGameLogic->getFrame() % (60 * LOGICFRAMES_PER_SECOND) >= secondsToFrames(config().aiThinkSeconds))
		return;

	std::string list;
	for (size_t t = 0; t < targets.size(); ++t)
		list += " " + std::to_string(targets[t]) + "(" + std::to_string(units.committed[t]) + ")";
	CoH::log("Sectors: frame %u AI %d (%s): $%u, %d structures, %d build spots refused, %d capturers, %d idle; targets [sector(units)]%s",
		TheGameLogic->getFrame(), player->getPlayerIndex(), player->getSide().str(), player->getMoney()->countMoney(),
		units.structures, refusedBuildSpots(player->getPlayerIndex()), units.capturers, (int)units.idle.size(), list.c_str());
}

void thinkFor(Player *player, const World &w)
{
	const PlayerId me = player->getPlayerIndex();
	std::vector<int> targets = rankTargets(me, viewFor(me, w), w.map().neighbors(), w.connectedOf(me), baseOf(me, w),
		config().ai);
	if ((int)targets.size() > config().aiMaxTargets)
		targets.resize(std::max(0, config().aiMaxTargets));

	Units units = findUnits(player, w, targets);
	logThinking(player, targets, units);
	for (size_t t = 0; t < targets.size() && !units.idle.empty(); ++t)
	{
		const int missing = config().aiGroupSize - units.committed[t];
		if (missing > 0)
			sendNearest(units.idle, w.sector(targets[t]).position, missing);
	}
}

} // namespace

void thinkForAIs()
{
	const World &w = world();
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		if (!player->isSkirmishAIPlayer() || !player->isPlayerActive())
			continue;
		adaptAIBuildList(player);
		thinkFor(player, w);
	}
}

} // namespace CoH::Sectors
