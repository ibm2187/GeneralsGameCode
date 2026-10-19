// CoH DirectionalArmor: how the slice's Engine and UI parts reach each other. Other slices use
// DirectionalArmor.h.

#pragma once

#include "Kernel/Hooks.h"

#include "Features/DirectionalArmor/Domain/Armor.h"

class Object;
class WeaponTemplate;
struct Coord3D;

namespace CoH
{
class Feature;
}

namespace CoH::DirectionalArmor
{

// DirectionalArmor.cpp
const Feature *armorFeature();	///< null before init

// Combat.cpp: the penetration roll and the damage against infantry, from the hub's weaponDamage.
void dealWeaponDamage(Hooks::WeaponDamage &hit);
// A tank shell's high-explosive blast against the soldiers beside its target, from weaponDetonated.
void detonateBlast(const Object *source, const WeaponTemplate *weapon, const Coord3D &position, float radius);
int blastsSoFar();		///< shell blasts this game, for the self-test
void logTally();		///< this game's rolls so far, to CoH.log
void forgetTally();

// Flanking.cpp: the skirmish AI's vehicles drive around armored targets.
void flankWithAI();
void logFlanking();
void forgetFlanking();

// SelfTest.cpp: -cohArmorSelfTest, from the logic frame.
void runSelfTest();
void resetSelfTest();

// UI/Deflection.cpp: sparks, a clang and "Deflected" where a shot bounced off. Only an effect.
void showDeflection(const Object &victim, Flat towardsShot, float penetration);

} // namespace CoH::DirectionalArmor
