// IBM Engine: armor, the functions the engine's hooks call. With the Armor feature on, vehicles have
// front, side and rear armor and weapons have penetration, like in Company of Heroes. A direct hit on
// a vehicle penetrates with chance penetration / armor (of the side it hits); a shot that doesn't is
// deflected and does only a small share of its damage. The IbmArmor INI block (in the Armor pack)
// sets every number.

#pragma once

class INI;
class Object;
class WeaponTemplate;

namespace IbmEngine
{

// WeaponTemplate::dealDamageInternal, before each victim takes damage: rolls penetration and
// returns the share of the damage that gets through (1 when it penetrates, or the hit doesn't roll).
// hitter is the object that deals the damage: the shooter, or its projectile, whose position gives
// the side that is hit. Only direct hits (the weapon's target, from a weapon with no real blast)
// roll; area damage always gets through. Always 1 when the feature is off.
Real rollArmorPenetration(const WeaponTemplate *weapon, const Object *hitter, const Object *victim, Bool isDirectHit);

// INI block "IbmArmor": the settings, read from the Armor pack at game start.
void parseArmorDefinition(INI *ini);

} // namespace IbmEngine
