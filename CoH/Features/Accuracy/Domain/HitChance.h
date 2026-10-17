// CoH Accuracy: the rules of a hit roll, as pure functions: which weapons roll and from what base,
// the built-in modifiers, the clamp, the roll and where a miss lands. Plain C++, unit tested; the
// numbers come from the CoHAccuracy block of the pack.

#pragma once

#include "Features/Accuracy/Domain/ModifierStack.h"
#include "Features/Accuracy/Domain/Shot.h"

namespace CoH::Accuracy
{

struct Rules
{
	float minChance = 0.05f;	///< the stacked chance is kept between these
	float maxChance = 0.95f;

	// Range bands, as a share of the weapon's range: near up to nearRange, far beyond farRange.
	float nearRange = 0.33f;
	float farRange = 0.67f;
	float nearMultiplier = 1.15f;
	float midMultiplier = 1.0f;
	float farMultiplier = 0.85f;

	float shooterMovingMultiplier = 0.75f;
	float targetMovingMultiplier = 0.85f;
	float sizeMultiplier[(int)TargetSize::Count] = { 0.9f, 1.0f, 0.9f, 1.5f };	///< by TargetSize
	float veterancyPerRank = 0.15f;	///< added to the multiplier for each rank

	float maxDamageRadius = 8.0f;	///< a weapon with a bigger blast is an area weapon and doesn't roll
	float missDistanceMin = 4.0f;	///< how far beyond the target's edge a miss lands
	float missDistanceMax = 15.0f;
};

// ---- Which weapons roll -------------------------------------------------------------------------

// What decides whether a weapon rolls and from what base, read from its template.
struct WeaponFacts
{
	int classPercent = -1;		///< the base accuracy of its damage type; -1 for a type that never rolls
	int weaponPercent = -1;		///< the weapon's own base accuracy; -1 when it has none
	float damageRadius = 0.0f;
	float scatterRadius = 0.0f;	///< weapons that scatter (artillery) are already inaccurate
	bool contact = false;		///< melee
	bool laser = false;
	bool damageAtSelf = false;	///< goes off where the shooter stands
};

// The weapon's base accuracy, 0..1, or a negative value when it doesn't roll. A weapon's own base
// wins over its class; 100% never misses, so it doesn't roll either.
float baseAccuracy(const Rules &rules, const WeaponFacts &weapon);

// ---- The built-in modifiers ----------------------------------------------------------------------

HitModifier rangeModifier(const Rules &rules, const Shot &shot);
HitModifier shooterMovingModifier(const Rules &rules, const Shot &shot);
HitModifier targetMovingModifier(const Rules &rules, const Shot &shot);
HitModifier targetSizeModifier(const Rules &rules, const Shot &shot);
HitModifier veterancyModifier(const Rules &rules, const Shot &shot);

// ---- The chance and the roll ---------------------------------------------------------------------

// The base with the stack applied, kept between the rules' min and max chance.
float hitChance(const Rules &rules, float baseChance, const ModifierStack &modifiers, const Shot &shot);

// A roll is a whole number from 0 to RollSides - 1, drawn from the synced random numbers.
constexpr int RollSides = 10000;

// Whether the roll hits at this chance: always at 1 or more, never at 0 or less.
bool isHit(float chance, int roll);

// Whether the chance needs a roll at all; a certain hit or miss draws no random number.
bool needsRoll(float chance);

// ---- Where a miss lands --------------------------------------------------------------------------

struct Offset
{
	float x = 0.0f;
	float y = 0.0f;
};

// The miss lands 'angle' radians around the target, beyond its edge (targetRadius) by between the
// rules' min and max miss distance; 'spread' 0..1 picks where in between.
Offset missOffset(const Rules &rules, float targetRadius, float angle, float spread);

} // namespace CoH::Accuracy
