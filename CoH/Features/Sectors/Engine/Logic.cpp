// CoH Sectors: the game logic of sectors, run on the logic frame. Capture: who is inside each point,
// and Domain/Capture moves it. A point that changes hands takes its tech buildings with it. Income:
// each player is paid for its connected sectors.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameUtility.h"
#include "Common/Money.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Team.h"
#include "GameClient/GameText.h"
#include "GameClient/InGameUI.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/PartitionManager.h"

#include "Kernel/Log.h"

#include "Features/Sectors/Engine/Config.h"
#include "Features/Sectors/Engine/Internal.h"
#include "Features/Sectors/Engine/SectorPointUpdate.h"
#include "Features/Sectors/Engine/World.h"

namespace CoH::Sectors
{

bool isCapturer(const Object *object)
{
	const Config &c = config();
	if (!object->isAnyKindOf(c.captureKinds) || object->isAnyKindOf(c.nonCaptureKinds))
		return false;
	if (object->isEffectivelyDead() || object->isContained() || object->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION))
		return false;
	if (object->isDisabledByType(DISABLED_UNMANNED))
		return false;

	const Player *owner = object->getControllingPlayer();
	return owner && owner->isPlayableSide() && owner->isPlayerActive();
}

namespace
{

std::vector<Presence> countCapturers(const Vec2 &position)
{
	std::vector<Presence> presences;
	const Coord3D center = { position.x, position.y, 0.0f };
	ObjectIterator *iter = ThePartitionManager->iterateObjectsInRange(&center, config().captureRadius, FROM_CENTER_2D);
	MemoryPoolObjectHolder hold(iter);

	for (Object *object = iter->first(); object; object = iter->next())
	{
		if (!isCapturer(object))
			continue;

		const PlayerId player = object->getControllingPlayer()->getPlayerIndex();
		auto it = std::find_if(presences.begin(), presences.end(), [player](const Presence &p) { return p.player == player; });
		if (it == presences.end())
			presences.push_back({ player, 1 });
		else
			++it->units;
	}
	return presences;
}

Team *teamOf(PlayerId owner)
{
	Player *player = findPlayer(owner);
	if (player == nullptr || !player->isPlayerActive())
		player = ThePlayerList->getNeutralPlayer();
	return player->getDefaultTeam();
}

// The point's tech buildings belong to whoever holds the point.
void handOverMembers(const SectorPointUpdate &point)
{
	Team *team = teamOf(point.state().owner);
	for (ObjectID id : point.getMembers())
	{
		Object *member = TheGameLogic->findObjectByID(id);
		if (member == nullptr || member->isEffectivelyDead() || !member->isAnyKindOf(config().highValueKinds))
			continue;
		if (member->getTeam() != team)
			member->setTeam(team);
	}
}

const char *playerName(PlayerId player)
{
	static char name[64];
	Player *found = findPlayer(player);
	if (found == nullptr)
		return "nobody";
	snprintf(name, sizeof(name), "%ls", found->getPlayerDisplayName().str());
	return name;
}

} // namespace

void advancePoints()
{
	World &w = world();
	const AreEnemies enemies = areEnemies;
	const Real seconds = secondsToFrames(config().captureIntervalSeconds) / (Real)LOGICFRAMES_PER_SECOND;

	for (int i = 0; i < w.sectorCount(); ++i)
	{
		SectorPointUpdate *point = w.point(i);
		if (point == nullptr)
			continue;

		const Occupation occupation = resolveOccupation(countCapturers(w.sector(i).position), enemies);
		w.setOccupation(i, occupation);

		const CaptureChange change = advanceCapture(point->state(), occupation, w.isLocked(i), config().capture, seconds, enemies);
		if (!change.ownerChanged)
			continue;

		handOverMembers(*point);
		announceOwnerChange(i, change.previousOwner, point->state().owner);
	}

	w.refreshNetwork();
}

// Also reconciles the tech buildings now and then: a building taken with a capture ability goes back.
void payIncome()
{
	World &w = world();
	for (int i = 0; i < w.sectorCount(); ++i)
	{
		if (const SectorPointUpdate *point = w.point(i))
			handOverMembers(*point);
	}

	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		const int income = w.incomePerPeriod(i);
		if (income > 0 && player->isPlayerActive())
			player->getMoney()->deposit((UnsignedInt)income, FALSE);
	}
}

void announceOwnerChange(int sector, int previousOwner, int newOwner)
{
	CoH::log("Sectors: frame %u sector %d (%.0f, %.0f) %s -> %s", TheGameLogic->getFrame(), sector,
		world().sector(sector).position.x, world().sector(sector).position.y, playerName(previousOwner), playerName(newOwner));

	const Player *local = rts::getObservedOrLocalPlayer();
	if (TheInGameUI == nullptr || local == nullptr)
		return;

	const PlayerId me = local->getPlayerIndex();
	if (newOwner == me)
		TheInGameUI->messageNoFormat(TheGameText->FETCH_OR_SUBSTITUTE("CoH:SectorCaptured", L"Sector captured"));
	else if (previousOwner == me)
		TheInGameUI->messageNoFormat(TheGameText->FETCH_OR_SUBSTITUTE("CoH:SectorLost", L"Sector lost"));
}

} // namespace CoH::Sectors
