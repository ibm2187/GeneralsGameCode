// CoH Squads: the slice's declaration and its hook subscribers. See CoH/Features/Squads/README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Kernel/Kernel.h"

#include "Features/Squads/Engine/Config.h"
#include "Features/Squads/Engine/Internal.h"

namespace
{

using namespace CoH::Squads;

void onGameStartAfterMap(bool /*loadingSaveGame*/)
{
	startGame();
	startProduction();
}

void onGameEnd()
{
	endGame();
	forgetProduction();
}

void onLogicFrame()
{
	carryOutOrders();
}

void init(CoH::Feature &self)
{
	CoH::registerIniBlock("CoHSquads", parseConfigBlock);

	CoH::Hooks::gameStartAfterMap().subscribe(self, onGameStartAfterMap);
	CoH::Hooks::gameEnd().subscribe(self, onGameEnd);
	CoH::Hooks::logicFrame().subscribe(self, onLogicFrame);
	CoH::Hooks::objectDestroyed().subscribe(self, onObjectDestroyed);

	CoH::Hooks::unitProduced().subscribe(self, onUnitProduced);
	CoH::Hooks::buildCost().subscribe(self, adjustBuildCost);
	CoH::Hooks::buildTime().subscribe(self, adjustBuildTime);
	CoH::Hooks::playerSelection().subscribe(self, onPlayerSelection);
	CoH::Hooks::aiCommand().subscribe(self, onAiCommand);
	CoH::Hooks::containRoom().subscribe(self, onContainRoom);
	CoH::Hooks::healthBar().subscribe(self, onHealthBar);
}

void reset()
{
	endGame();
	forgetProduction();
	resetConfig();
}

} // namespace

COH_FEATURE(Squads, init, reset)
