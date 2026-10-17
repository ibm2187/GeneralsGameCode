// CoH GeneralsArsenal: see Cooldown.h.

#include "Cooldown.h"

#include <algorithm>
#include <cmath>

namespace CoH::GeneralsArsenal
{

unsigned int readyFrameAfterLoss(unsigned int readyFrame, unsigned int now, unsigned int penaltyFrames,
	unsigned int cooldown, unsigned int maxDelay)
{
	const unsigned int from = std::max(readyFrame, now);
	return std::max(from, std::min(from + penaltyFrames, now + cooldown + maxDelay));
}

unsigned int secondsToFrames(float seconds, int framesPerSecond)
{
	if (seconds <= 0.0f || framesPerSecond <= 0)
		return 0;
	return (unsigned int)std::ceil(seconds * framesPerSecond);
}

} // namespace CoH::GeneralsArsenal
