// CoH DirectionalArmor: what other slices may ask about vehicle armor. The only header of this slice
// others include.
//
// Vehicles have front, side and rear armor and weapons have penetration. A hit on an armored vehicle
// penetrates with chance penetration / armor of the side it hits; a shot that doesn't is deflected and
// does a small share of its damage. Vehicles' tank guns and machine guns hit infantry harder. Every
// answer here is "no armor" while the DirectionalArmor flag is off.
//
// Damage order for one victim (CoH/Docs/CONTEXT.md §4, Combat): the hit roll (Accuracy, at the fire),
// cover (Cover, the hub's weaponDamage), then this slice (weaponDamage, after Cover), then the engine's
// armor, then veterancy (the hub's damageReceived).

#pragma once

#include <optional>

class Object;
class WeaponTemplate;
struct Coord3D;

namespace CoH::DirectionalArmor
{

// The game being played has directional armor.
bool isActive();

// The object rolls penetration when hit (a vehicle with armor in this game).
bool hasArmor(const Object &object);

// The chance from 0 to 1 that a shot of 'weapon' fired from 'from' gets through 'target''s armor, or
// nothing when the shot wouldn't roll (no armor, a weapon without penetration, the flag off). For
// Vehicle criticals (08) and AI targeting.
std::optional<float> penetrationChanceFrom(const Object &target, const WeaponTemplate &weapon, const Coord3D &from);

} // namespace CoH::DirectionalArmor
