// CoH DirectionalArmor: see Flank.h.

#include "Features/DirectionalArmor/Domain/Flank.h"

#include <cmath>

namespace CoH::DirectionalArmor
{

namespace
{

Flat minus(Flat a, Flat b)
{
	return { a.x - b.x, a.y - b.y };
}

float length(Flat v)
{
	return std::sqrt(v.x * v.x + v.y * v.y);
}

Flat unit(Flat v)
{
	const float l = length(v);
	return l > 0.0f ? Flat{ v.x / l, v.y / l } : Flat{};
}

} // namespace

std::optional<Flat> flankSpot(const FlankSituation &s, const FlankRules &rules)
{
	const Flat forward = unit(s.targetFacing);
	const Flat toAttacker = minus(s.attacker, s.target);
	if (length(forward) <= 0.0f || sideHit(forward, toAttacker, s.arcs) != Side::Front)
		return std::nullopt;

	const float chanceNow = penetrationChance(penetrationAt(s.penetration, length(toAttacker), s.range), s.armor.front);
	if (chanceNow >= rules.belowChance)
		return std::nullopt;

	// Beside the target on the attacker's side of it, swung back by rearShare.
	const Flat left = { -forward.y, forward.x };
	const float sign = (toAttacker.x * left.x + toAttacker.y * left.y) >= 0.0f ? 1.0f : -1.0f;
	const Flat away = unit({ left.x * sign * (1.0f - rules.rearShare) - forward.x * rules.rearShare,
		left.y * sign * (1.0f - rules.rearShare) - forward.y * rules.rearShare });
	const float distance = s.range * rules.rangeShare;
	const Flat spot = { s.target.x + away.x * distance, s.target.y + away.y * distance };

	const Side sideThere = sideHit(forward, away, s.arcs);
	const float chanceThere = penetrationChance(penetrationAt(s.penetration, distance, s.range), s.armor.of(sideThere));
	if (expectedDamage(chanceThere, sideThere, s.hits) - expectedDamage(chanceNow, Side::Front, s.hits) < rules.minGain)
		return std::nullopt;
	if (length(minus(spot, s.attacker)) > rules.maxTravel)
		return std::nullopt;
	return spot;
}

} // namespace CoH::DirectionalArmor
