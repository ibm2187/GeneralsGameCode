// CoH GeneralsArsenal: when a General's power may be used. Every power has a command point cost, a
// cooldown (its special power's ReloadTime) and a minimum rank, all set in the pack. Rank opens the
// tiers (ranks 1, 3 and 5); command points pay for each use. Air powers also need an airfield.

#pragma once

#include <string>
#include <vector>

namespace CoH::GeneralsArsenal
{

// How the skirmish AI uses a power its scripts don't know.
enum class AiUse
{
	None,			///< the AI's own scripts use it, or nobody does
	Strike,			///< at the most valuable enemy group or base
	Cover,			///< over the AI's own army
	Superweapon,	///< at the most valuable enemy base
};

// A level of a power (A-10 strike 1, 2 and 3): its science, and the rank it comes with.
struct PowerLevel
{
	std::string science;
	int rank = 1;
};

struct PowerRule
{
	int cost = 0;						///< command points per use
	int minRank = 1;
	bool needsAirfield = false;
	float lossPenaltySeconds = 0.0f;	///< added to the cooldown for each of its planes shot down
	AiUse aiUse = AiUse::None;
	std::vector<PowerLevel> levels;		///< the first is granted at the start, the others at their rank
};

// What a power's button shows, in the order they are checked.
enum class PowerState
{
	Ready,
	LockedByRank,
	NeedsAirfield,
	CoolingDown,
	TooExpensive,
};

struct PlayerStanding
{
	int rank = 1;
	int points = 0;
	bool hasAirfield = false;
	bool coolingDown = false;
};

PowerState stateOf(const PowerRule &rule, const PlayerStanding &player);

// The cooldown is the engine's own (the button's clock), so "usable" leaves it out.
bool isAllowed(const PowerRule &rule, const PlayerStanding &player);

// The sciences of 'levels' a player of 'rank' should have, in order: the first always, the others
// once their rank is reached. A level waits for the one before it.
std::vector<std::string> sciencesDue(const std::vector<PowerLevel> &levels, int rank);

} // namespace CoH::GeneralsArsenal
