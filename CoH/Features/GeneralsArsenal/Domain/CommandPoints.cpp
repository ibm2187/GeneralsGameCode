// CoH GeneralsArsenal: see CommandPoints.h.

#include "CommandPoints.h"

#include <algorithm>

namespace CoH::GeneralsArsenal
{

float earnedForKill(int experienceValue, const EarnRates &rates)
{
	if (experienceValue <= 0)
		return 0.0f;
	return experienceValue * rates.perKillExperience;
}

float earnedForDamage(float damage, float maxHealth, int experienceValue, const EarnRates &rates)
{
	if (damage <= 0.0f || maxHealth <= 0.0f || experienceValue <= 0)
		return 0.0f;
	const float share = std::min(damage, maxHealth) / maxHealth;
	return share * experienceValue * rates.perDamageExperience;
}

void Wallet::earn(float amount, float maxPoints)
{
	if (amount <= 0.0f || m_points >= maxPoints)
		return;
	m_points = std::min(m_points + amount, maxPoints);
}

bool Wallet::spend(int cost)
{
	if (cost <= 0)
		return true;
	if (!canAfford(cost))
		return false;
	m_points -= (float)cost;
	return true;
}

void Wallet::charge(int cost)
{
	if (cost > 0)
		m_points = std::max(0.0f, m_points - (float)cost);
}

} // namespace CoH::GeneralsArsenal
