// CoH FireRate: the slice's declaration, its hook subscriber and its public answers. See
// CoH/Features/FireRate/README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/Weapon.h"

#include "Kernel/Kernel.h"

#include "Features/FireRate/Engine/Config.h"
#include "Features/FireRate/FireRate.h"

namespace
{

using namespace CoH::FireRate;

const CoH::Feature *&theFeature()
{
	static const CoH::Feature *feature = nullptr;
	return feature;
}

// Every bonus the weapon fires with passes here, so the slower fire and the bigger damage apply to
// its shots, reloads, projectiles going off and the AI's estimates alike. Other bonuses (veterancy,
// upgrades) still multiply on top.
void slowWeapon(const Object * /*source*/, const WeaponTemplate *weapon, WeaponBonus &bonus)
{
	const Scaling &scaling = weaponScaling(weapon);
	if (!scaling.changes())
		return;
	bonus.setField(WeaponBonus::RATE_OF_FIRE, bonus.getField(WeaponBonus::RATE_OF_FIRE) / (Real)scaling.slowDown);
	bonus.setField(WeaponBonus::DAMAGE, bonus.getField(WeaponBonus::DAMAGE) * (Real)scaling.damage);
}

void init(CoH::Feature &self)
{
	theFeature() = &self;
	CoH::registerIniBlock("CoHFireRate", parseConfigBlock);
	CoH::Hooks::weaponBonus().subscribe(self, slowWeapon);
}

void reset()
{
	resetConfig();
}

} // namespace

namespace CoH::FireRate
{

bool isActive()
{
	return theFeature() != nullptr && theFeature()->isActive();
}

float slowDown(const WeaponTemplate *weapon)
{
	return isActive() ? (float)weaponScaling(weapon).slowDown : 1.0f;
}

} // namespace CoH::FireRate

COH_FEATURE(FireRate, init, reset)
