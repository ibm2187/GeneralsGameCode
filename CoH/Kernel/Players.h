// CoH Kernel: players and who is whose enemy, for C++20 feature code. The player lists are views over
// ThePlayerList, in its order (the same on every machine), and never hold a null player:
//
//     for (Player *player : CoH::activeSides())
//         payIncome(*player);
//
//     auto enemies = CoH::activeSides()
//         | std::views::filter([&](const Player *other) { return CoH::isEnemy(*me, *other); });
//
// Relationships are one-way, as in the engine: isEnemy(a, b) is how a sees b.

#pragma once

#include "Common/Player.h"
#include "Common/PlayerList.h"

#include <ranges>

class Object;

namespace CoH
{

// Every player, the neutral and civilian ones too.
inline auto players()
{
	return std::views::iota(Int(0), ThePlayerList->getPlayerCount())
		| std::views::transform([](Int index) { return ThePlayerList->getNthPlayer(index); })
		| std::views::filter([](const Player *player) { return player != nullptr; });
}

// The players that play a side: humans and AIs, not neutral or civilian.
inline auto sides()
{
	return players() | std::views::filter([](const Player *player) { return player->isPlayableSide(); });
}

// The sides still in the game: not defeated, not observing.
inline auto activeSides()
{
	return sides() | std::views::filter([](const Player *player) { return player->isPlayerActive(); });
}

// The skirmish AI sides still in the game.
inline auto aiSides()
{
	return activeSides() | std::views::filter([](Player *player) { return player->isSkirmishAIPlayer(); });
}

bool isEnemy(const Player &viewer, const Player &other);
bool isAlly(const Player &viewer, const Player &other);
bool isEnemy(const Object &viewer, const Object &other);
bool isAlly(const Object &viewer, const Object &other);

} // namespace CoH
