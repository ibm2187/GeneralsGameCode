// CoH Sectors: the plain types every Domain rule shares. No engine headers, so the rules are unit tested.

#pragma once

#include <functional>

namespace CoH::Sectors
{

// A player's index in the engine's player list. Domain rules only compare them.
using PlayerId = int;
constexpr PlayerId NoPlayer = -1;

// What a sector is worth and how it behaves.
enum class SectorKind
{
	Supply,		///< made from supply docks, warehouses and piles
	HighValue,	///< has an oil derrick or another tech building
	Home,		///< a player's start position; owned from the first frame
};

struct Vec2
{
	float x = 0.0f;
	float y = 0.0f;
};

inline float distanceSquared(Vec2 a, Vec2 b)
{
	const float dx = a.x - b.x;
	const float dy = a.y - b.y;
	return dx * dx + dy * dy;
}

// Whether two players fight each other. The engine answers it from its relationships.
using AreEnemies = std::function<bool(PlayerId, PlayerId)>;

} // namespace CoH::Sectors
