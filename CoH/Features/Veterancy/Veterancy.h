// CoH Veterancy: what other slices may ask about a unit's rank. The only header of this slice others
// include.
//
// The engine's ranks (Veteran, Elite, Heroic) are earned from damage dealt as well as kills, a squad
// shares one rank, and each rank's bonuses are set per unit type in the pack: accuracy (through
// Accuracy's "Veterancy" modifier), damage taken, suppression resistance, speed, sight and reload.
// Everything here answers "no bonus" while the Veterancy flag is off.

#pragma once

class Object;

namespace CoH::Veterancy
{

// The game being played uses CoH veterancy.
bool isActive();

// The share of suppression the unit shrugs off at its rank, 0..1: Suppression (06) scales what it
// would add by (1 - this). 0 while the flag is off.
float suppressionResistance(const Object *unit);

// What the unit's rank multiplies its hit chance and the damage it takes by. 1 while the flag is off.
float accuracyMultiplier(const Object *unit);
float damageTakenMultiplier(const Object *unit);

} // namespace CoH::Veterancy
