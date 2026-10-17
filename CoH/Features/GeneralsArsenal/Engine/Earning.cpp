// CoH GeneralsArsenal: command points earned in combat. A kill earns a share of the victim's experience
// value (what a veteran unit would gain for it), and damage earns a share of that value by the part
// of the victim's health it took, so points come in all through a fight and not only on kills.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Player.h"
#include "GameLogic/Damage.h"
#include "GameLogic/ExperienceTracker.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Module/BodyModule.h"
#include "GameLogic/Object.h"

#include "Features/GeneralsArsenal/Engine/Config.h"
#include "Features/GeneralsArsenal/Engine/Internal.h"

namespace CoH::GeneralsArsenal
{

namespace
{

int experienceValueOf(const Object *victim, const Object *attacker)
{
	const ExperienceTracker *tracker = victim->getExperienceTracker();
	return tracker ? tracker->getExperienceValue(attacker) : 0;
}

// The same as the engine credits a kill: an enemy of a playable side, not one being built.
bool isCreditable(const Object *attacker, const Object *victim)
{
	const Player *victimPlayer = victim->getControllingPlayer();
	const Player *attackerPlayer = attacker->getControllingPlayer();
	if (victimPlayer == nullptr || attackerPlayer == nullptr || victimPlayer == attackerPlayer)
		return false;
	if (!victimPlayer->isPlayableSide() || victim->isKindOf(KINDOF_IGNORED_IN_GUI))
		return false;
	if (victim->testStatus(OBJECT_STATUS_UNDER_CONSTRUCTION))
		return false;
	return attacker->getRelationship(victim) == ENEMIES;
}

} // namespace

// Object::scoreTheKill has checked the kill already.
void earnForKill(Object *killer, const Object *victim)
{
	Wallet *wallet = walletOf(killer->getControllingPlayer());
	if (wallet)
		wallet->earn(earnedForKill(experienceValueOf(victim, killer), config().earn), config().earn.maxPoints);
}

void earnForDamage(Object *victim, const DamageInfo *damage)
{
	if (damage == nullptr || damage->out.m_actualDamageClipped <= 0.0f)
		return;
	Object *attacker = TheGameLogic->findObjectByID(damage->in.m_sourceID);
	if (attacker == nullptr || !isCreditable(attacker, victim))
		return;
	Wallet *wallet = walletOf(attacker->getControllingPlayer());
	if (wallet == nullptr)
		return;

	const Real maxHealth = victim->getBodyModule() ? victim->getBodyModule()->getMaxHealth() : 0.0f;
	const float earned = earnedForDamage(damage->out.m_actualDamageClipped, maxHealth, experienceValueOf(victim, attacker),
		config().earn);
	wallet->earn(earned, config().earn.maxPoints);
}

} // namespace CoH::GeneralsArsenal
