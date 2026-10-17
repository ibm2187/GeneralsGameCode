// CoH GeneralsArsenal: the slice's declaration and its hook subscribers. See
// CoH/Features/GeneralsArsenal/README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Money.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"

#include "Kernel/Kernel.h"
#include "Kernel/Log.h"

#include "Features/GeneralsArsenal/Engine/AirCoverZoneUpdate.h"
#include "Features/GeneralsArsenal/Engine/CommandPointsLedger.h"
#include "Features/GeneralsArsenal/Engine/Config.h"
#include "Features/GeneralsArsenal/Engine/Internal.h"
#include "Features/GeneralsArsenal/Engine/SortieLossDie.h"
#include "Features/GeneralsArsenal/Engine/StrikeCountdownUpdate.h"

namespace
{

using namespace CoH::GeneralsArsenal;

bool isDue(UnsignedInt frame, Real seconds)
{
	return frame % secondsToLogicFrames(seconds) == 0;
}

void onGameStartAfterMap(bool loadingSaveGame)
{
	if (!loadingSaveGame)
		createLedger();
	refreshAirfields();
}

void logStanding()
{
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		if (player == nullptr || !player->isPlayableSide() || walletOf(player) == nullptr)
			continue;
		CoH::log("GeneralsArsenal: frame %u %ls (%s) has %.1f command points, rank %d, %s", TheGameLogic->getFrame(),
			player->getPlayerDisplayName().str(), player->getSide().str(), walletOf(player)->points(), player->getRankLevel(),
			hasAirfield(player) ? "an airfield" : "no airfield");
	}
}

// -cohUnlock, for trying the powers: every human player is rank 5, has $100,000 at the start, and
// command points kept full. Test games only: a replay played without it goes its own way.
void unlockForTesting(bool atStart)
{
	for (Int i = 0; i < ThePlayerList->getPlayerCount(); ++i)
	{
		Player *player = ThePlayerList->getNthPlayer(i);
		Wallet *wallet = walletOf(player);
		if (player == nullptr || wallet == nullptr || player->getPlayerType() != PLAYER_HUMAN || !player->isPlayableSide())
			continue;
		if (atStart)
		{
			player->setRankLevel(5);
			player->getMoney()->deposit(100000, FALSE, FALSE);
		}
		wallet->set(config().earn.maxPoints);
	}
}

void onLogicFrame()
{
	const UnsignedInt frame = TheGameLogic->getFrame();
	static const bool unlock = CoH::hasCommandLineOption("-cohUnlock");
	if (unlock && isDue(frame, 1.0f))
		unlockForTesting(frame == 0);
	if (isDue(frame, 1.0f))
	{
		refreshAirfields();
		grantPowerSciences();
	}
	thinkForAIs();
	if (CoH::isLogging() && frame > 0 && isDue(frame, 60.0f))
	{
		logStanding();
		logAIPowers();
	}
}

void onObjectCreated(Object *object)
{
	tagPlane(object);
}

void onGameEnd()
{
	forgetLastTrigger();
	releaseMarkerText();
}

void init(CoH::Feature &self)
{
	CoH::registerIniBlock("CoHGeneralsArsenal", parseConfigBlock);
	CoH::registerIniBlock("CoHPower", parsePowerBlock);
	COH_ADD_MODULE(CommandPointsLedger);
	COH_ADD_MODULE(SortieLossDie);
	COH_ADD_MODULE(StrikeCountdownUpdate);
	COH_ADD_MODULE(AirCoverZoneUpdate);

	CoH::Hooks::gameStartAfterMap().subscribe(self, onGameStartAfterMap);
	CoH::Hooks::logicFrame().subscribe(self, onLogicFrame);
	CoH::Hooks::objectCreated().subscribe(self, onObjectCreated);
	CoH::Hooks::gameEnd().subscribe(self, onGameEnd);

	CoH::Hooks::killScored().subscribe(self, earnForKill);
	CoH::Hooks::damageDealt().subscribe(self, earnForDamage);
	CoH::Hooks::specialPowerUse().subscribe(self, allowsPower);
	CoH::Hooks::specialPowerTriggered().subscribe(self, payForPower);

	CoH::Hooks::moneyDisplay().subscribe(self, addPointsToMoneyText);
	CoH::Hooks::moneyTooltip().subscribe(self, addPointsToMoneyTooltip);
	CoH::Hooks::commandTooltip().subscribe(self, addPowerToTooltip);
	CoH::Hooks::inGameUIPostDraw().subscribe(self, drawMarkersOnGround);
	CoH::Hooks::radarDraw().subscribe(self, drawMarkersOnRadar);
}

void reset()
{
	forgetLastTrigger();
	resetConfig();
}

} // namespace

COH_FEATURE(GeneralsArsenal, init, reset)
