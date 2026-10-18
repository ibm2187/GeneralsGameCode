// CoH Accuracy: the rules of a hit roll, as pure functions: which weapons roll, their accuracy at
// a range, the built-in modifiers, the clamp, the roll and where a miss lands. Plain C++, unit
// tested; the numbers come from the CoHAccuracy block of the pack.

#pragma once

#include "Features/Accuracy/Domain/ModifierStack.h"
#include "Features/Accuracy/Domain/Shot.h"

#include <vector>

namespace CoH::Accuracy
{

// A weapon's accuracy at point-blank (near), at half its range (mid) and at its full range (far),
// 0..1 or more; in between it changes in a straight line. Above 1 is allowed: a sniper's 1.5 still
// hits small targets often once the size multiplier has taken its share.
struct RangeProfile
{
	float nearAccuracy = -1.0f;	///< negative: no profile
	float midAccuracy = -1.0f;
	float farAccuracy = -1.0f;

	bool isSet() const { return nearAccuracy >= 0.0f; }
};

// The target's size multiplier at a bounding radius, in world units. Between points it changes in a
// straight line; below the first and above the last it stays at that point's value.
struct SizePoint
{
	float radius = 0.0f;
	float multiplier = 1.0f;
};

struct Rules
{
	float minChance = 0.05f;			///< the stacked chance is kept between these...
	float maxChance = 0.90f;			///< ...against units
	float maxChanceBuilding = 0.95f;	///< ...against buildings: the easiest targets, never certain

	std::vector<SizePoint> sizeCurve = { { 7.0f, 0.6f }, { 10.0f, 0.7f }, { 16.0f, 0.9f }, { 20.0f, 1.0f },
		{ 30.0f, 1.15f }, { 45.0f, 1.4f } };	///< by radius; kept sorted by radius
	float buildingSizeMin = 1.8f;		///< a building's size multiplier is at least this, however small it is

	float shooterMovingMultiplier = 0.75f;
	float targetMovingMultiplier = 0.85f;
	float veterancyPerRank = 0.15f;	///< added to the multiplier for each rank

	float maxDamageRadius = 8.0f;	///< a weapon with a bigger blast is an area weapon and doesn't roll
	float missDistanceMin = 4.0f;	///< how far beyond the target's edge a miss lands
	float missDistanceMax = 15.0f;
};

// ---- Which weapons roll, and how well at a range ---------------------------------------------------

// What decides whether a weapon rolls and with what profile, read from its template.
struct WeaponFacts
{
	RangeProfile classProfile;	///< of its damage type; not set for a type that never rolls
	RangeProfile ownProfile;	///< the weapon's own; not set when it has none
	bool neverMisses = false;	///< listed to never miss
	float damageRadius = 0.0f;
	float scatterRadius = 0.0f;	///< weapons that scatter (artillery) are already inaccurate
	bool contact = false;		///< melee
	bool laser = false;
	bool damageAtSelf = false;	///< goes off where the shooter stands
};

// The profile the weapon rolls with, or one not set when it doesn't roll. The weapon's own profile
// wins over its class's.
RangeProfile rollingProfile(const Rules &rules, const WeaponFacts &weapon);

// The profile's accuracy at 'distance' of a weapon whose range is 'range'.
float accuracyAt(const RangeProfile &profile, float distance, float range);

// ---- The built-in modifiers ----------------------------------------------------------------------

float sizeMultiplier(const Rules &rules, TargetSize size, float radius);

HitModifier shooterMovingModifier(const Rules &rules, const Shot &shot);
HitModifier targetMovingModifier(const Rules &rules, const Shot &shot);
HitModifier targetSizeModifier(const Rules &rules, const Shot &shot);
HitModifier veterancyModifier(const Rules &rules, const Shot &shot);

// ---- The chance and the roll ---------------------------------------------------------------------

// The base with the stack applied, kept between the rules' min and the max for the target.
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
