// CoH Accuracy: see HitChance.h.

#include "Features/Accuracy/Domain/HitChance.h"

#include <algorithm>
#include <cmath>

namespace CoH::Accuracy
{

namespace
{

float lerp(float from, float to, float t)
{
	return from + (to - from) * t;
}

} // namespace

RangeProfile rollingProfile(const Rules &rules, const WeaponFacts &weapon)
{
	if (weapon.neverMisses || weapon.contact || weapon.laser || weapon.damageAtSelf)
		return {};
	if (weapon.scatterRadius > 0.0f || weapon.damageRadius > rules.maxDamageRadius)
		return {};
	return weapon.ownProfile.isSet() ? weapon.ownProfile : weapon.classProfile;
}

float accuracyAt(const RangeProfile &profile, float distance, float range)
{
	if (range <= 0.0f)
		return profile.midAccuracy;

	const float share = std::clamp(distance / range, 0.0f, 1.0f);
	if (share <= 0.5f)
		return lerp(profile.nearAccuracy, profile.midAccuracy, share * 2.0f);
	return lerp(profile.midAccuracy, profile.farAccuracy, (share - 0.5f) * 2.0f);
}

float sizeMultiplier(const Rules &rules, TargetSize size, float radius)
{
	const std::vector<SizePoint> &curve = rules.sizeCurve;
	float multiplier = 1.0f;
	if (!curve.empty())
	{
		const auto above = std::find_if(curve.begin(), curve.end(),
			[radius](const SizePoint &point) { return point.radius > radius; });
		if (above == curve.begin())
			multiplier = curve.front().multiplier;
		else if (above == curve.end())
			multiplier = curve.back().multiplier;
		else
		{
			const SizePoint &below = *(above - 1);
			multiplier = lerp(below.multiplier, above->multiplier, (radius - below.radius) / (above->radius - below.radius));
		}
	}

	if (size == TargetSize::Building)
		multiplier = std::max(multiplier, rules.buildingSizeMin);
	return multiplier;
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
	return scaleChance(sizeMultiplier(rules, shot.targetSize, shot.targetRadius));
}

HitModifier veterancyModifier(const Rules &rules, const Shot &shot)
{
	return scaleChance(1.0f + rules.veterancyPerRank * std::max(shot.veterancy, 0));
}

float hitChance(const Rules &rules, float baseChance, const ModifierStack &modifiers, const Shot &shot)
{
	const float chance = modifiers.apply(baseChance, shot);
	const float maxChance = shot.targetSize == TargetSize::Building ? rules.maxChanceBuilding : rules.maxChance;
	return std::clamp(chance, rules.minChance, std::max(rules.minChance, maxChance));
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
