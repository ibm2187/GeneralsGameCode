// CoH FireRate: what other slices may ask about slowed weapons. The only header of this slice others
// include.
//
// A weapon that fires faster than its class allows fires slower, and each shot does more damage by
// the same factor, so a fight turns on fewer, bigger shots (and fewer Accuracy rolls). Every answer
// here is "unchanged" while the FireRate flag is off.

#pragma once

class WeaponTemplate;

namespace CoH::FireRate
{

// The game being played slows fast weapons.
bool isActive();

// How many times longer the weapon's delays and reloads are in this game; 1 when it isn't slowed.
// Its shots do this many times the damage (with DamagePerSecond at 100%).
float slowDown(const WeaponTemplate *weapon);

} // namespace CoH::FireRate
