// CoH Accuracy: the slice's declaration and its hook subscribers. See CoH/Features/Accuracy/README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/GameLogic.h"

#include "Kernel/Kernel.h"
#include "Kernel/Log.h"

#include "Features/Accuracy/Engine/Config.h"
#include "Features/Accuracy/Engine/Internal.h"

namespace
{

using namespace CoH::Accuracy;

const CoH::Feature *&theFeature()
{
	static const CoH::Feature *feature = nullptr;
	return feature;
}

void logTally(const char *when)
{
	const Tally &counted = tally();
	CoH::log("Accuracy: %s, %d hits of %d rolls (%d%%)", when, counted.hits, counted.rolls,
		counted.rolls > 0 ? counted.hits * 100 / counted.rolls : 0);
}

void onGameStartAfterMap(bool /*loadingSaveGame*/)
{
	forgetTally();
}

void onLogicFrame()
{
	const UnsignedInt frame = TheGameLogic->getFrame();
	if (CoH::isLogging() && frame > 0 && frame % (60 * LOGICFRAMES_PER_SECOND) == 0)
	{
		AsciiString when;
		when.format("frame %u", frame);
		logTally(when.str());
	}
}

void onGameEnd()
{
	if (CoH::isLogging())
		logTally("game over");
	forgetTally();
}

void init(CoH::Feature &self)
{
	theFeature() = &self;
	CoH::registerIniBlock("CoHAccuracy", parseConfigBlock);
	addBuiltInModifiers(self);

	CoH::Hooks::weaponFiringAtObject().subscribe(self, rollShot);
	CoH::Hooks::gameStartAfterMap().subscribe(self, onGameStartAfterMap);
	CoH::Hooks::logicFrame().subscribe(self, onLogicFrame);
	CoH::Hooks::gameEnd().subscribe(self, onGameEnd);
}

void reset()
{
	forgetTally();
	resetConfig();
}

} // namespace

namespace CoH::Accuracy
{

const Feature *accuracyFeature()
{
	return theFeature();
}

} // namespace CoH::Accuracy

COH_FEATURE(Accuracy, init, reset)
