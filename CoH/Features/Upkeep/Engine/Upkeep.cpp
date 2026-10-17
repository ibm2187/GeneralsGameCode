// CoH Upkeep: the slice's declaration and its hook subscribers. See CoH/Features/Upkeep/README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Money.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "GameLogic/GameLogic.h"

#include "Kernel/Kernel.h"
#include "Kernel/Log.h"

#include "Features/Sectors/Sectors.h"
#include "Features/Upkeep/Engine/Config.h"
#include "Features/Upkeep/Engine/Internal.h"

namespace
{

using namespace CoH::Upkeep;

// Set while a game with the flag on is played: the subscribers that set it run only then.
bool &flagOnThisGame()
{
	static bool theFlag = false;
	return theFlag;
}

void logStanding()
{
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		const Player *player = ThePlayerList->getNthPlayer(i);
		if (player == nullptr || !player->isPlayerActive() || player == ThePlayerList->getNeutralPlayer())
			continue;
		const Standing standing = standingOf(player);
		if (standing.population == 0 && standing.buildingBonus == 0)
			continue;	// the civilians
		CoH::log("Upkeep: frame %u player %d population %d of %d (buildings +%d, sectors %d), upkeep %d%%, taken $%d so far, money $%u",
			TheGameLogic->getFrame(), i, standing.population, standing.cap, standing.buildingBonus,
			CoH::Sectors::heldSectors(player), standing.percent, upkeepTaken(i), player->getMoney()->countMoney());
	}
}

void onGameStartAfterMap(bool /*loadingSaveGame*/)
{
	flagOnThisGame() = true;
	forgetUpkeepTaken();
}

void onLogicFrame()
{
	const UnsignedInt frame = TheGameLogic->getFrame();
	if (CoH::isLogging() && isUpkeepGame() && frame > 0 && frame % (60 * LOGICFRAMES_PER_SECOND) == 0)
		logStanding();
}

void onGameEnd()
{
	flagOnThisGame() = false;
	forgetUpkeepTaken();
}

void init(CoH::Feature &self)
{
	CoH::registerIniBlock("CoHUpkeep", parseConfigBlock);

	CoH::Hooks::gameStartAfterMap().subscribe(self, onGameStartAfterMap);
	CoH::Hooks::logicFrame().subscribe(self, onLogicFrame);
	CoH::Hooks::gameEnd().subscribe(self, onGameEnd);
	CoH::Hooks::production().subscribe(self, allowsProduction);
	CoH::Hooks::income().subscribe(self, takeUpkeep);

	CoH::Hooks::moneyDisplay().subscribe(self, addPopulationToMoneyText);
	CoH::Hooks::moneyTooltip().subscribe(self, addUpkeepToMoneyTooltip);
	CoH::Hooks::commandTooltip().subscribe(self, addPopulationToTooltip);
}

void reset()
{
	flagOnThisGame() = false;
	forgetUpkeepTaken();
	resetConfig();
}

} // namespace

namespace CoH::Upkeep
{

// Population needs sectors: the cap grows with them, and upkeep is a share of their income.
bool isUpkeepGame()
{
	return flagOnThisGame() && Sectors::isActive();
}

} // namespace CoH::Upkeep

COH_FEATURE(Upkeep, init, reset)
