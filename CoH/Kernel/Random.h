// CoH Kernel: synced random numbers for C++20 feature code, over GameLogicRandomValue. Every machine
// and every replay draws the same numbers in the same order, so game logic stays in sync:
//
//     if (CoH::chance(config.hitChance))
//         hit(*target);
//     if (Object **target = CoH::pick(targets))	// targets is a std::vector<Object *>
//         attack(**target);
//
// Game logic only. Each draw moves the sequence, so never draw in UI or drawing code, or just to log.
// Visuals use GameClientRandomValue instead.

#pragma once

#include "GameLogic/LogicRandomValue.h"

#include "Kernel/Domain/Chance.h"

#include <ranges>

namespace CoH
{

// True with 'probability', from 0 to 1. It always draws one number, whatever the probability, so how
// many numbers a feature draws never depends on its tuning.
inline bool chance(Real probability)
{
	return rollPasses(GameLogicRandomValueReal(0.0f, 1.0f), probability);
}

// A whole number from 'lo' to 'hi', both included. It draws nothing when lo >= hi and returns lo.
inline Int between(Int lo, Int hi)
{
	return lo < hi ? GameLogicRandomValue(lo, hi) : lo;
}

// One of the items, each as likely, or nullptr when there are none: a std::span, std::vector, std::array
// or plain array. It draws nothing for none or one. The items must outlive the pointer, so a temporary
// vector doesn't compile.
template <std::ranges::contiguous_range Items>
	requires std::ranges::sized_range<Items> && std::ranges::borrowed_range<Items>
auto pick(Items &&items) -> decltype(std::ranges::data(items))
{
	const auto count = static_cast<Int>(std::ranges::size(items));
	if (count == 0)
		return nullptr;
	return std::ranges::data(items) + between(0, count - 1);
}

} // namespace CoH
