// CoH Sectors: see World.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "Common/Team.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/TerrainLogic.h"

#include "Features/Sectors/Domain/Network.h"
#include "Features/Sectors/Engine/Config.h"
#include "Features/Sectors/Engine/SectorPointUpdate.h"
#include "Features/Sectors/Engine/World.h"

#include <algorithm>

namespace CoH::Sectors
{

namespace
{

bool isStandingCommandCenter(const Object *object)
{
	return object->isKindOf(KINDOF_COMMANDCENTER) && !object->isEffectivelyDead() &&
		!object->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION);
}

} // namespace

World &world()
{
	static World theWorld;
	return theWorld;
}

Player *findPlayer(PlayerId player)
{
	if (player < 0 || player >= ThePlayerList->getPlayerCount())
		return nullptr;
	return ThePlayerList->getNthPlayer(player);
}

bool areEnemies(PlayerId a, PlayerId b)
{
	if (a == b)
		return false;
	const Player *first = findPlayer(a);
	const Player *second = findPlayer(b);
	return first && second && first->getRelationship(second->getDefaultTeam()) == ENEMIES;
}

Vec2 toVec2(const Coord3D &position)
{
	return { position.x, position.y };
}

void World::clear()
{
	*this = World();
}

bool World::build()
{
	clear();
	m_built = true;
	static int builds = 0;
	m_buildNumber = ++builds;

	std::vector<Object *> points;
	for (Object *object = TheGameLogic->getFirstObject(); object; object = object->getNextObject())
	{
		if (SectorPointUpdate::find(object))
			points.push_back(object);
	}
	if (points.empty())
		return false;

	// In the order the points were made, so a loaded game gets the same sector numbers.
	std::sort(points.begin(), points.end(), [](const Object *a, const Object *b) { return a->getID() < b->getID(); });

	std::vector<Site> sites;
	for (Object *object : points)
	{
		const SectorPointUpdate *point = SectorPointUpdate::find(object);
		m_sectors.push_back({ object->getID(), toVec2(*object->getPosition()), point->getKind(), point->getHomePlayer() });
		sites.push_back({ toVec2(*object->getPosition()), point->getKind() == SectorKind::Home });
	}

	Region3D extent;
	TheTerrainLogic->getExtent(&extent);
	m_map = SectorMap::build(sites, extent.hi.x, extent.hi.y, config().map);
	m_occupation.resize(m_sectors.size());
	refreshNetwork();
	return true;
}

SectorPointUpdate *World::point(int index) const
{
	return SectorPointUpdate::find(TheGameLogic->findObjectByID(m_sectors[index].point));
}

PlayerId World::ownerOf(int index) const
{
	const SectorPointUpdate *module = point(index);
	return module ? module->state().owner : NoPlayer;
}

std::vector<PlayerId> World::owners() const
{
	std::vector<PlayerId> list;
	for (int i = 0; i < sectorCount(); ++i)
		list.push_back(ownerOf(i));
	return list;
}

int World::sectorAt(const Coord3D &position) const
{
	return m_map.sectorAt(toVec2(position));
}

std::vector<int> World::rootsOf(PlayerId player) const
{
	std::vector<int> roots;
	for (int i = 0; i < sectorCount(); ++i)
	{
		if (m_sectors[i].kind == SectorKind::Home && m_sectors[i].homePlayer == player)
			roots.push_back(i);
	}

	if (player >= 0 && player < (int)m_commandCenterSectors.size())
		roots.insert(roots.end(), m_commandCenterSectors[player].begin(), m_commandCenterSectors[player].end());
	return roots;
}

bool World::isLocked(int index) const
{
	const Sector &home = m_sectors[index];
	if (home.kind != SectorKind::Home || ownerOf(index) != home.homePlayer)
		return false;

	return home.homePlayer >= 0 && home.homePlayer < (int)m_hasCommandCenter.size() && m_hasCommandCenter[home.homePlayer];
}

void World::findCommandCenters()
{
	const Int players = ThePlayerList->getPlayerCount();
	m_hasCommandCenter.assign(players, false);
	m_commandCenterSectors.assign(players, std::vector<int>());

	for (Object *object = TheGameLogic->getFirstObject(); object; object = object->getNextObject())
	{
		const Player *owner = object->getControllingPlayer();
		if (owner == nullptr || !isStandingCommandCenter(object))
			continue;

		const Int index = owner->getPlayerIndex();
		m_hasCommandCenter[index] = true;
		m_commandCenterSectors[index].push_back(sectorAt(*object->getPosition()));
	}
}

void World::refreshNetwork()
{
	findCommandCenters();

	m_owners = owners();
	std::vector<SectorKind> kinds;
	for (const Sector &sector : m_sectors)
		kinds.push_back(sector.kind);

	const Int players = ThePlayerList->getPlayerCount();
	m_connected.assign(players, std::vector<bool>());
	m_income.assign(players, 0);
	for (Int player = 0; player < players; ++player)
	{
		m_connected[player] = findConnected(m_map.neighbors(), m_owners, player, rootsOf(player));
		m_income[player] = incomeFor(kinds, m_connected[player], config().income);
	}
}

const std::vector<bool> &World::connectedOf(PlayerId player) const
{
	static const std::vector<bool> none;
	return player >= 0 && player < (int)m_connected.size() ? m_connected[player] : none;
}

bool World::isConnected(PlayerId player, int index) const
{
	const std::vector<bool> &connected = connectedOf(player);
	return index >= 0 && index < (int)connected.size() && connected[index];
}

bool World::isCutOff(int index) const
{
	const PlayerId owner = ownerOf(index);
	return owner != NoPlayer && !isConnected(owner, index);
}

int World::incomePerPeriod(PlayerId player) const
{
	return player >= 0 && player < (int)m_income.size() ? m_income[player] : 0;
}

int World::incomePerMinute(PlayerId player) const
{
	return Sectors::incomePerMinute(incomePerPeriod(player), config().incomeSeconds);
}

} // namespace CoH::Sectors
