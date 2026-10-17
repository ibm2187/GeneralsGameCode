// CoH Accuracy: see HitChance.h.

#include "Features/Accuracy/Domain/HitChance.h"

#include <algorithm>
#include <cmath>

namespace CoH::Accuracy
{

float baseAccuracy(const Rules &rules, const WeaponFacts &weapon)
{
	if (weapon.contact || weapon.laser || weapon.damageAtSelf)
		return -1.0f;
	if (weapon.scatterRadius > 0.0f || weapon.damageRadius > rules.maxDamageRadius)
		return -1.0f;

	const int percent = weapon.weaponPercent >= 0 ? weapon.weaponPercent : weapon.classPercent;
	if (percent < 0 || percent >= 100)
		return -1.0f;
	return percent / 100.0f;
}

HitModifier rangeModifier(const Rules &rules, const Shot &shot)
{
	if (shot.range <= 0.0f)
		return scaleChance(rules.midMultiplier);

	const float share = shot.distance / shot.range;
	if (share <= rules.nearRange)
		return scaleChance(rules.nearMultiplier);
	if (share > rules.farRange)
		return scaleChance(rules.farMultiplier);
	return scaleChance(rules.midMultiplier);
}

HitModifier shooterMovingModifier(const Rules &rules, const Shot &shot)
{
	return scaleChance(shot.shooterMoving ? rules.shooterMovingMultiplier : 1.0f);
}

HitModifier targetMovingModifier(const Rules &rules, const Shot &shot)
{
	return scaleChance(shot.targetMoving ? rules.targetMovingMultiplier : 1.0f);
}

HitModifier targetSizeModifier(const Rules &rules, const Shot &shot)
{
	const int size = (int)shot.targetSize;
	if (size < 0 || size >= (int)TargetSize::Count)
		return {};
	return scaleChance(rules.sizeMultiplier[size]);
}

HitModifier veterancyModifier(const Rules &rules, const Shot &shot)
{
	return scaleChance(1.0f + rules.veterancyPerRank * std::max(shot.veterancy, 0));
}

float hitChance(const Rules &rules, float baseChance, const ModifierStack &modifiers, const Shot &shot)
{
	const float chance = modifiers.apply(baseChance, shot);
	return std::clamp(chance, rules.minChance, std::max(rules.minChance, rules.maxChance));
}

namespace
{

// The rolls below this hit.
int hitThreshold(float chance)
{
	return (int)std::lround(std::clamp(chance, 0.0f, 1.0f) * RollSides);
}

} // namespace

bool isHit(float chance, int roll)
{
	return roll < hitThreshold(chance);
}

bool needsRoll(float chance)
{
	const int threshold = hitThreshold(chance);
	return threshold > 0 && threshold < RollSides;
}

Offset missOffset(const Rules &rules, float targetRadius, float angle, float spread)
{
	const float beyondEdge = rules.missDistanceMin
		+ std::clamp(spread, 0.0f, 1.0f) * std::max(0.0f, rules.missDistanceMax - rules.missDistanceMin);
	const float distance = std::max(targetRadius, 0.0f) + beyondEdge;
	return { distance * std::cos(angle), distance * std::sin(angle) };
}

} // namespace CoH::Accuracy
