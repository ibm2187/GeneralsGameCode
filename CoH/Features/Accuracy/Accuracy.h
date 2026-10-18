// CoH Accuracy: direct fire rolls to hit. The only header of this slice others include.
//
// Every shot of a direct-fire weapon at an object rolls against a hit chance: the weapon's accuracy
// at that range (its base) with the hit modifiers applied in order, kept between a min and a max. A
// miss lands on the ground near the target. Area weapons and artillery don't roll.
//
// Other slices change the chance with hit modifiers of their own, added in their init:
//
//     CoH::Accuracy::HitModifier inCover(const CoH::Accuracy::Shot &shot) { ... return CoH::Accuracy::scaleChance(0.7f); }
//
//     void init(CoH::Feature &self)
//     {
//         CoH::Accuracy::addHitModifier(self, "Cover", CoH::Accuracy::ModifierOrder::Cover, inCover);
//     }
//
// A modifier counts only while its feature (and Accuracy) is on. The built-in modifiers are named
// "ShooterMoving", "TargetMoving", "TargetSize" and "Veterancy". A modifier added under one
// of those names stands in for the built-in one while its feature is on.

#pragma once

#include "Features/Accuracy/Domain/Shot.h"

namespace CoH
{
class Feature;
}

namespace CoH::Accuracy
{

// The game being played rolls to hit.
bool isActive();

// Adds a hit modifier for the feature 'owner' (the Feature its init is given). See the top of this
// file and ModifierOrder in Domain/Shot.h. Call it in init, once.
void addHitModifier(const Feature &owner, const char *name, int order, ModifierFunction modifier);

} // namespace CoH::Accuracy
