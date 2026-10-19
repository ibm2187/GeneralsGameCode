// CoH DirectionalArmor: when the skirmish AI drives around a vehicle to hit its side or rear, and to
// where, as pure code. Unit tested in Tests/FlankTest.cpp.

#pragma once

#include "Features/DirectionalArmor/Domain/Armor.h"

#include <optional>

namespace CoH::DirectionalArmor
{

struct FlankRules
{
	float belowChance = 0.75f;	///< flank only while a hit from here gets through less often than this
	float minGain = 0.15f;		///< and a hit from the flank spot does at least this share more damage on average
	float rangeShare = 0.8f;	///< the spot is this share of the attacker's range from the target
	float rearShare = 0.4f;		///< 0 is straight beside the target, 1 straight behind it
	float maxTravel = 250.0f;	///< never drive further than this to get there
};

struct FlankSituation
{
	Flat target;			///< where the target is
	Flat targetFacing;		///< where its hull points
	Flat attacker;			///< where the attacker is
	Arcs arcs;
	ArmorValues armor;		///< the target's
	Penetration penetration;	///< the attacker's weapon
	float range = 0.0f;		///< the attacker's weapon
	HitRules hits;			///< what deflections and each side's hits do
};

// Where the attacker should go to hit the target from the side or rear, on the side it is already on,
// or nothing when it shouldn't: it already hits the side or rear, it gets through the front well
// enough, the flank gains too little, or the drive is too long.
std::optional<Flat> flankSpot(const FlankSituation &situation, const FlankRules &rules);

} // namespace CoH::DirectionalArmor
