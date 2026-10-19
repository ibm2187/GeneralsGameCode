// CoH DirectionalArmor: see Armor.h.

#include "Features/DirectionalArmor/Domain/Armor.h"

#include <algorithm>
#include <cmath>

namespace CoH::DirectionalArmor
{

namespace
{

constexpr float Pi = 3.14159265358979f;

// cos(x) for x from 0 to pi/2: the Taylor series to x^14, plain multiplies and adds only.
float cosUpToRightAngle(float x)
{
	const float xx = x * x;
	float term = 1.0f;
	float sum = 1.0f;
	for (int n = 2; n <= 14; n += 2)
	{
		term *= -xx / (float)((n - 1) * n);
		sum += term;
	}
	return sum;
}

float length(Flat v)
{
	return std::sqrt(v.x * v.x + v.y * v.y);	// sqrt is exactly rounded everywhere, unlike cos
}

} // namespace

float cosOfDegrees(float degrees)
{
	float d = degrees < 0.0f ? -degrees : degrees;
	while (d > 360.0f)
		d -= 360.0f;
	if (d > 180.0f)
		d = 360.0f - d;
	if (d > 90.0f)
		return -cosUpToRightAngle((180.0f - d) * Pi / 180.0f);
	return cosUpToRightAngle(d * Pi / 180.0f);
}

Arcs arcsFromDegrees(float frontWidth, float rearWidth)
{
	const float front = std::clamp(frontWidth, 0.0f, 360.0f);
	const float rear = std::clamp(rearWidth, 0.0f, 360.0f);
	return { .frontCos = cosOfDegrees(front / 2.0f), .rearCos = cosOfDegrees(rear / 2.0f) };
}

Side sideHit(Flat facing, Flat toShooter, const Arcs &arcs)
{
	const float lengths = length(facing) * length(toShooter);
	if (lengths <= 0.0f)
		return Side::Front;

	// The cosine of the angle between where the vehicle points and where the shot came from.
	const float cosine = (facing.x * toShooter.x + facing.y * toShooter.y) / lengths;
	if (cosine >= arcs.frontCos)
		return Side::Front;
	if (-cosine >= arcs.rearCos)
		return Side::Rear;
	return Side::Side;
}

const char *sideName(Side side)
{
	switch (side)
	{
		case Side::Front: return "front";
		case Side::Side: return "side";
		case Side::Rear: return "rear";
	}
	return "?";
}

float ArmorValues::of(Side hit) const
{
	switch (hit)
	{
		case Side::Front: return front;
		case Side::Side: return side;
		case Side::Rear: return rear;
	}
	return front;
}

float SideDamage::of(Side hit) const
{
	switch (hit)
	{
		case Side::Front: return front;
		case Side::Side: return side;
		case Side::Rear: return rear;
	}
	return front;
}

float expectedDamage(float chance, Side side, const HitRules &rules)
{
	const float through = std::clamp(chance, 0.0f, 1.0f);
	return (through + (1.0f - through) * damageScale(false, rules.deflectedShare)) * rules.sideDamage.of(side);
}

float penetrationAt(const Penetration &penetration, float distance, float range)
{
	if (range <= 0.0f)
		return penetration.atNear;
	const float along = std::clamp(distance / range, 0.0f, 1.0f);
	return penetration.atNear + (penetration.atFar - penetration.atNear) * along;
}

float penetrationChance(float penetration, float armor)
{
	if (armor <= 0.0f)
		return 1.0f;
	return std::clamp(penetration / armor, 0.0f, 1.0f);
}

float damageScale(bool penetrated, float deflectedShare)
{
	return penetrated ? 1.0f : std::clamp(deflectedShare, 0.0f, 1.0f);
}

} // namespace CoH::DirectionalArmor
