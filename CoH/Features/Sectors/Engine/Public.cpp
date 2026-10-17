// CoH Sectors: the answers of the public header, Sectors.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"

#include "Features/Sectors/Engine/World.h"
#include "Features/Sectors/Sectors.h"

namespace CoH::Sectors
{

namespace
{

int countSectors(const Player *player, bool connectedOnly)
{
	const World &w = world();
	if (!w.hasSectors() || player == nullptr)
		return 0;

	const PlayerId me = player->getPlayerIndex();
	int count = 0;
	for (int i = 0; i < w.sectorCount(); ++i)
	{
		if (w.ownersAtRefresh()[i] == me && (!connectedOnly || w.isConnected(me, i)))
			++count;
	}
	return count;
}

} // namespace

bool isActive()
{
	return world().hasSectors();
}

int ownerIndexAt(const Coord3D &position)
{
	const World &w = world();
	if (!w.hasSectors())
		return NoPlayer;
	const int sector = w.sectorAt(position);
	return sector >= 0 ? w.ownersAtRefresh()[sector] : NoPlayer;
}

int heldSectors(const Player *player)
{
	return countSectors(player, false);
}

int connectedSectors(const Player *player)
{
	return countSectors(player, true);
}

int incomePerMinute(const Player *player)
{
	return player && world().hasSectors() ? world().incomePerMinute(player->getPlayerIndex()) : 0;
}

} // namespace CoH::Sectors
