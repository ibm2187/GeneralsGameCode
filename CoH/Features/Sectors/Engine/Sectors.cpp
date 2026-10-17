// CoH Sectors: the slice's declaration and its hook subscribers. See CoH/Features/Sectors/README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"

#include "Kernel/Kernel.h"
#include "Kernel/Log.h"

#include "Features/Sectors/Engine/Config.h"
#include "Features/Sectors/Engine/Internal.h"
#include "Features/Sectors/Engine/SectorPointUpdate.h"
#include "Features/Sectors/Engine/World.h"

namespace
{

using namespace CoH::Sectors;

bool isDue(UnsignedInt frame, Real seconds)
{
	return frame % secondsToFrames(seconds) == 0;
}

void logStanding()
{
	const World &w = world();
	for (int i = 0; i < w.sectorCount(); ++i)
	{
		const SectorPointUpdate *point = w.point(i);
		if (point == nullptr)
			continue;
		CoH::log("Sectors: frame %u sector %d owner %d progress %.2f by %d%s%s", TheGameLogic->getFrame(), i,
			point->state().owner, point->state().progress, point->state().progressSide,
			w.occupation(i).kind == Occupation::Kind::Contested ? " contested" : "", w.isCutOff(i) ? " cut off" : "");
	}
	for (int player = 0; player < MAX_PLAYER_COUNT; ++player)
	{
		if (w.incomePerPeriod(player) > 0)
			CoH::log("Sectors: frame %u player %d earns %d per minute", TheGameLogic->getFrame(), player, w.incomePerMinute(player));
	}
}

// The world is made on the first frame: for a new game the points were just placed, and a loaded
// game's points are in only once its objects have loaded.
void buildWorldOnce()
{
	World &w = world();
	if (w.isBuilt())
		return;
	if (w.build())
		CoH::log("Sectors: %d sectors, %d borders", w.sectorCount(), (int)w.map().borders().size());
}

void onGameStartAfterMap(bool loadingSaveGame)
{
	world().clear();
	if (!isSectorGame())
		return;

	retargetAIScripts();
	if (!loadingSaveGame)
		setUpNewGame();
}

void onLogicFrame()
{
	buildWorldOnce();
	if (!world().hasSectors())
		return;

	const UnsignedInt frame = TheGameLogic->getFrame();
	if (isDue(frame, config().captureIntervalSeconds))
		advancePoints();
	if (frame > 0 && isDue(frame, config().incomeSeconds))
		payIncome();
	if (isDue(frame, config().aiThinkSeconds))
		thinkForAIs();
	if (CoH::isLogging() && frame > 0 && isDue(frame, 60.0f))
		logStanding();
}

// Supplies that appear later (a script, a crate) can't be gathered either.
void onObjectCreated(Object *object)
{
	if (world().hasSectors())
		emptySupplies(object);
}

void onGameEnd()
{
	world().clear();
	forgetRefusedBuildSpots();
}

bool allowsBuilding(const ThingTemplate *build, const Coord3D *position, const Player *player)
{
	return allowsBuildingAt(build, position, player);
}

void init(CoH::Feature &self)
{
	CoH::registerIniBlock("CoHSectors", parseConfigBlock);
	COH_ADD_MODULE(SectorPointUpdate);

	CoH::Hooks::gameStartAfterMap().subscribe(self, onGameStartAfterMap);
	CoH::Hooks::logicFrame().subscribe(self, onLogicFrame);
	CoH::Hooks::objectCreated().subscribe(self, onObjectCreated);
	CoH::Hooks::gameEnd().subscribe(self, onGameEnd);
	CoH::Hooks::buildLocation().subscribe(self, allowsBuilding);

	CoH::Hooks::inGameUIPostDraw().subscribe(self, drawGround);
	CoH::Hooks::radarDraw().subscribe(self, drawRadar);
	CoH::Hooks::moneyTooltip().subscribe(self, addIncomeToTooltip);
}

void reset()
{
	world().clear();
	resetConfig();
}

} // namespace

COH_FEATURE(Sectors, init, reset)
