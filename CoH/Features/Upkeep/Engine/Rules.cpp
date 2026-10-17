// CoH Upkeep: the cap on production and the upkeep taken from income.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "Common/ThingTemplate.h"

#include "Features/Upkeep/Engine/Config.h"
#include "Features/Upkeep/Engine/Internal.h"

namespace CoH::Upkeep
{

namespace
{

int (&takenByPlayer())[MAX_PLAYER_COUNT]
{
	static int theTaken[MAX_PLAYER_COUNT] = {};
	return theTaken;
}

} // namespace

// A unit that doesn't fit under the cap can't be made. The AI also stops before upkeep passes its
// limit, so most of its income stays free for rebuilding and its economy never stalls.
bool allowsProduction(const Player *player, const ThingTemplate *build)
{
	if (!isUpkeepGame() || player == nullptr || isNeverRefused(build))
		return true;
	const int cost = unitPopulation(build);
	if (cost <= 0)
		return true;

	const Standing standing = standingOf(player);
	if (!fitsUnderCap(standing.population, cost, standing.cap))
		return false;
	if (player->getPlayerType() == PLAYER_COMPUTER &&
		!aiMayGrowTo(standing.population + cost, config().steps, config().aiMaxUpkeepPercent))
		return false;
	return true;
}

void takeUpkeep(const Player *player, unsigned int &amount)
{
	if (!isUpkeepGame())
		return;
	const IncomeSplit split = splitIncome((int)amount, standingOf(player).percent);
	if (split.upkeep <= 0)
		return;

	const int index = player->getPlayerIndex();
	if (index >= 0 && index < MAX_PLAYER_COUNT)
		takenByPlayer()[index] += split.upkeep;
	amount = (unsigned int)split.net;
}

int upkeepTaken(int playerIndex)
{
	return playerIndex >= 0 && playerIndex < MAX_PLAYER_COUNT ? takenByPlayer()[playerIndex] : 0;
}

void forgetUpkeepTaken()
{
	for (int &taken : takenByPlayer())
		taken = 0;
}

} // namespace CoH::Upkeep
