// CoH Kernel: what a random roll means, as pure code. Kernel/Random.h draws the rolls.

#pragma once

namespace CoH
{

// Whether a roll from 0 to 1 (both included) passes 'probability': 0 or less never does, 1 or more
// always does.
constexpr bool rollPasses(float roll, float probability)
{
	return probability >= 1.0f || roll < probability;
}

} // namespace CoH
