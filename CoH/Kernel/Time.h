// CoH Kernel: game time for C++20 feature code. Game logic measures time in logic frames, never with
// the clock, so replays and multiplayer stay in sync. The conversions are pure (Domain/Frames.h):
//
//     const UnsignedInt delay = CoH::framesFromSeconds(config.delaySeconds);
//     if (CoH::everySeconds(2.0f, player->getPlayerIndex()))	// each player on its own frame
//         think(*player);

#pragma once

#include "Common/GameCommon.h"
#include "GameLogic/GameLogic.h"

#include "Kernel/Domain/Frames.h"

static_assert(CoH::FramesPerSecond == LOGICFRAMES_PER_SECOND, "Domain/Frames.h must match the engine");

namespace CoH
{

// The current logic frame: the same on every machine.
inline UnsignedInt now()
{
	return TheGameLogic->getFrame();
}

// The frame 'seconds' from now, rounded up to a whole frame.
inline UnsignedInt secondsFromNow(Real seconds)
{
	return now() + framesFromSeconds(seconds);
}

// Whether this frame starts a new period of 'seconds'; 'offset' frames shift it (see isEvery).
inline bool everySeconds(Real seconds, UnsignedInt offset = 0)
{
	return isEvery(now(), framesFromSeconds(seconds), offset);
}

} // namespace CoH
