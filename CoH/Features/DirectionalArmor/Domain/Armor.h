// CoH DirectionalArmor: which side a shot hits, how hard it pushes through, and what a deflected shot
// does, as pure code. No engine headers: unit tested in Tests/ArmorTest.cpp.
//
// A vehicle has front, side and rear armor; a weapon has penetration at point-blank and at its full
// range, with a straight line in between. A shot penetrates with chance penetration / armor (always,
// when penetration is at least the armor), the way Company of Heroes rolls it. A shot that doesn't is
// deflected and does only a share of its damage.

#pragma once

#include <concepts>

namespace CoH::DirectionalArmor
{

// A direction or offset on the ground. Need not be unit length.
struct Flat
{
	float x = 0.0f;
	float y = 0.0f;
};

enum class Side
{
	Front,
	Side,
	Rear
};

// The cosine of an angle in degrees, from plain arithmetic (a Taylor series), so every machine gets the
// same bits, unlike the library's cos. Accurate to about 1e-6 from 0 to 180 degrees; outside, mirrored.
float cosOfDegrees(float degrees);

// The front and rear arcs, from their full widths in degrees (90 is 45 degrees either side). The sides
// are what is left. Widths over 360 are 360; front and rear may not overlap (the front wins).
struct Arcs
{
	float frontCos = 0.70710677f;	///< a shot this close to straight ahead (or closer) hits the front
	float rearCos = 0.70710677f;	///< a shot this close to straight behind (or closer) hits the rear
};
Arcs arcsFromDegrees(float frontWidth, float rearWidth);

// Which side a shot from 'toShooter' (from the vehicle to where the shot came from) hits a vehicle
// facing 'facing'. A shooter right on top of the vehicle (no direction) hits the front.
Side sideHit(Flat facing, Flat toShooter, const Arcs &arcs);

const char *sideName(Side side);

struct ArmorValues
{
	float front = 0.0f;
	float side = 0.0f;
	float rear = 0.0f;

	float of(Side hit) const;
};

// Penetration at point-blank and at the weapon's full range. ("near" and "far" are macros in the
// Windows headers, so the members can't use those names.)
struct Penetration
{
	float atNear = 0.0f;
	float atFar = 0.0f;
};

// The penetration at 'distance' for a weapon of 'range': near at 0, far at the range and beyond. A
// weapon with no range uses near.
float penetrationAt(const Penetration &penetration, float distance, float range);

// The chance a shot of 'penetration' gets through 'armor', from 0 to 1. No armor never stops it.
float penetrationChance(float penetration, float armor);

// What a hit does to the damage: all of it on a penetration, 'deflectedShare' of it on a deflection.
float damageScale(bool penetrated, float deflectedShare);

// The whole roll for one hit. 'passes' rolls a chance from 0 to 1 and says whether it came up: the
// Kernel's synced CoH::chance in game, anything in tests. It is asked once per hit, whatever the chance.
struct Hit
{
	Side side = Side::Front;
	float penetration = 0.0f;
	float armor = 0.0f;
	float chance = 1.0f;
	bool penetrated = true;
	float damageScale = 1.0f;
};

template <std::predicate<float> Roll>
Hit resolveHit(Side side, const ArmorValues &armor, float penetration, float deflectedShare, Roll &&passes)
{
	Hit hit;
	hit.side = side;
	hit.penetration = penetration;
	hit.armor = armor.of(side);
	hit.chance = penetrationChance(penetration, hit.armor);
	hit.penetrated = passes(hit.chance);
	hit.damageScale = damageScale(hit.penetrated, deflectedShare);
	return hit;
}

} // namespace CoH::DirectionalArmor
