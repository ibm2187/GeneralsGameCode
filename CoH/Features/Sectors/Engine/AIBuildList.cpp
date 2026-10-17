// CoH Sectors: the skirmish AI's build list, fitted to sectors. Its scripts keep adding to it during
// the game ("a supply center by the nearest supplies", "a barracks on the flank"), and a priority entry
// the AI can't place holds up everything else it would build. So on every AI step the supply buildings
// come out, and an unbuilt building planned where the build rule says no is pulled back toward the AI's
// home along the same line, to the farthest spot it may build.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/ThingFactory.h"
#include "Common/ThingTemplate.h"
#include "GameLogic/SidesList.h"

#include "Kernel/Log.h"

#include "Features/Sectors/Domain/Points.h"
#include "Features/Sectors/Engine/Config.h"
#include "Features/Sectors/Engine/Internal.h"
#include "Features/Sectors/Engine/World.h"

#include <algorithm>
#include <cmath>

namespace CoH::Sectors
{

namespace
{

// Takes the supply buildings out: there is nothing to gather with them.
void removeSupplyBuildings(Player *player)
{
	BuildListInfo *head = player->getBuildList();
	BuildListInfo *kept = nullptr;
	BuildListInfo *lastKept = nullptr;

	for (BuildListInfo *info = head; info;)
	{
		BuildListInfo *next = info->getNext();
		const ThingTemplate *plan = TheThingFactory->findTemplate(info->getTemplateName());
		const Bool remove = plan && plan->isAnyKindOf(config().aiBuildListRemovals);

		if (remove)
		{
			CoH::log("Sectors: %ls's AI no longer builds %s", player->getPlayerDisplayName().str(), info->getTemplateName().str());
			info->setNextBuildList(nullptr);
			if (info != head)
				deleteInstance(info);
		}
		else
		{
			if (lastKept)
				lastKept->setNextBuildList(info);
			else
				kept = info;
			lastKept = info;
		}
		info = next;
	}
	if (lastKept)
		lastKept->setNextBuildList(nullptr);

	// setBuildList deletes the old head, and only it once it is cut loose.
	if (kept != head)
		player->setBuildList(kept);
}

bool homeOf(PlayerId player, Vec2 &home)
{
	const World &w = world();
	for (int i = 0; i < w.sectorCount(); ++i)
	{
		if (w.sector(i).kind == SectorKind::Home && w.sector(i).homePlayer == player)
		{
			home = w.sector(i).position;
			return true;
		}
	}
	return false;
}

// The farthest spot from home toward 'planned' where the player may build; home if there is none.
Coord3D pullBack(const ThingTemplate *plan, const Coord3D &planned, Vec2 home, const Player *player)
{
	const Real dx = planned.x - home.x;
	const Real dy = planned.y - home.y;
	const Int steps = std::max(1, (Int)std::ceil(std::sqrt(dx * dx + dy * dy) / world().map().cellSize()));
	for (Int step = steps - 1; step > 0; --step)
	{
		const Coord3D spot = { home.x + dx * step / steps, home.y + dy * step / steps, planned.z };
		if (isBuildingAllowedAt(plan, &spot, player))
			return spot;
	}
	return { home.x, home.y, planned.z };
}

void pullBackIllegalPlans(Player *player)
{
	Vec2 home;
	if (!world().hasSectors() || !homeOf(player->getPlayerIndex(), home))
		return;

	for (BuildListInfo *info = player->getBuildList(); info; info = info->getNext())
	{
		if (info->getObjectID() != INVALID_ID)
			continue;
		const ThingTemplate *plan = TheThingFactory->findTemplate(info->getTemplateName());
		if (plan == nullptr || isBuildingAllowedAt(plan, info->getLocation(), player))
			continue;

		const Coord3D spot = pullBack(plan, *info->getLocation(), home, player);
		CoH::log("Sectors: AI %d moves its planned %s from (%.0f, %.0f) to (%.0f, %.0f)", player->getPlayerIndex(),
			info->getTemplateName().str(), info->getLocation()->x, info->getLocation()->y, spot.x, spot.y);
		info->setLocation(spot);
	}
}

// For the log: how far the AI's planned buildings spread from its start.
} // namespace

void logAIBuildList(Player *player, const std::vector<Start> &starts)
{
	if (!CoH::isLogging())
		return;
	for (const Start &start : starts)
	{
		if (start.player != player->getPlayerIndex())
			continue;
		int entries = 0;
		int outside = 0;
		Real farthest = 0.0f;
		for (BuildListInfo *info = player->getBuildList(); info; info = info->getNext())
		{
			const Real distance = std::sqrt(distanceSquared(toVec2(*info->getLocation()), start.position));
			++entries;
			outside += distance > config().map.homeRadius ? 1 : 0;
			farthest = std::max(farthest, distance);
		}
		CoH::log("Sectors: AI %d build list: %d buildings, %d beyond the home radius, farthest %.0f", start.player,
			entries, outside, farthest);
	}
}


void adaptAIBuildList(Player *player)
{
	removeSupplyBuildings(player);
	pullBackIllegalPlans(player);
}

} // namespace CoH::Sectors
