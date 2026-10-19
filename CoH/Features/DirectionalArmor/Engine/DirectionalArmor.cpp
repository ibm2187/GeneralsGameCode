// CoH DirectionalArmor: the slice's declaration, its hub subscriptions and its public answers. See
// CoH/Features/DirectionalArmor/README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/Object.h"
#include "GameLogic/Weapon.h"

#include "Kernel/Kernel.h"
#include "Kernel/Time.h"

#include "Features/DirectionalArmor/DirectionalArmor.h"
#include "Features/DirectionalArmor/Engine/Config.h"
#include "Features/DirectionalArmor/Engine/Internal.h"

#include <cmath>

namespace
{

using namespace CoH::DirectionalArmor;

const CoH::Feature *&theFeature()
{
	static const CoH::Feature *feature = nullptr;
	return feature;
}

void onLogicFrame()
{
	flankWithAI();
	runSelfTest();
	if (CoH::everySeconds(60.0f))
	{
		logTally();
		logFlanking();
	}
}

void onGameEnd()
{
	logTally();
	logFlanking();
}

void init(CoH::Feature &self)
{
	theFeature() = &self;
	CoH::registerIniBlock("CoHDirectionalArmor", parseConfigBlock);
	CoH::Hooks::weaponDetonated().subscribe(self, detonateBlast);
	CoH::Hooks::weaponDamage().subscribe(self, dealWeaponDamage);
	CoH::Hooks::logicFrame().subscribe(self, onLogicFrame);
	CoH::Hooks::gameEnd().subscribe(self, onGameEnd);
}

void reset()
{
	forgetTally();
	forgetFlanking();
	resetSelfTest();
	resetConfig();
}

} // namespace

namespace CoH::DirectionalArmor
{

const Feature *armorFeature()
{
	return theFeature();
}

bool isActive()
{
	return theFeature() != nullptr && theFeature()->isActive();
}

bool hasArmor(const Object &object)
{
	return isActive() && armorOf(object) != nullptr;
}

std::optional<float> penetrationChanceFrom(const Object &target, const WeaponTemplate &weapon, const Coord3D &from)
{
	if (!isActive())
		return std::nullopt;
	const ArmorValues *armor = armorOf(target);
	const Penetration *penetration = penetrationOf(weapon);
	if (armor == nullptr || penetration == nullptr)
		return std::nullopt;

	const Coord3D &at = *target.getPosition();
	const Flat towards = { from.x - at.x, from.y - at.y };
	const Coord3D &facing = *target.getUnitDirectionVector2D();
	const Side side = sideHit({ facing.x, facing.y }, towards, config().arcs);
	const WeaponBonus noBonus;
	const float distance = std::sqrt(towards.x * towards.x + towards.y * towards.y);
	return penetrationChance(penetrationAt(*penetration, distance, weapon.getAttackRange(noBonus)), armor->of(side));
}

} // namespace CoH::DirectionalArmor

COH_FEATURE(DirectionalArmor, init, reset)
