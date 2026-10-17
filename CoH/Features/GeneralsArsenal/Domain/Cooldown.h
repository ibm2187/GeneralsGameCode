// CoH GeneralsArsenal: a plane shot down lengthens the cooldown of the power that sent it, so air
// defense and escorts matter. Times are in logic frames.

#pragma once

namespace CoH::GeneralsArsenal
{

// The frame the power is ready again after losing a plane at 'now': the penalty goes on top of what
// is left of the cooldown, or on top of now when it is ready. Losses never push it more than
// 'maxDelay' past a full cooldown started now ('cooldown'), and a loss never makes it ready sooner.
unsigned int readyFrameAfterLoss(unsigned int readyFrame, unsigned int now, unsigned int penaltyFrames,
	unsigned int cooldown, unsigned int maxDelay);

unsigned int secondsToFrames(float seconds, int framesPerSecond);

} // namespace CoH::GeneralsArsenal
