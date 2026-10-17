// CoH GeneralsArsenal: command points, the per-player currency every General's power is paid with.
// They are earned all the time from combat: a share of the experience value of what the player's
// units damage and kill. Fractions are kept, so small hits add up.

#pragma once

namespace CoH::GeneralsArsenal
{

struct EarnRates
{
	float perKillExperience = 0.1f;		///< points per experience point of an enemy killed
	float perDamageExperience = 0.05f;	///< points per experience point, scaled by the share of its health taken
	float maxPoints = 1000.0f;			///< points stop growing here
};

// What killing an enemy worth 'experienceValue' earns.
float earnedForKill(int experienceValue, const EarnRates &rates);

// What taking 'damage' of an enemy's 'maxHealth' earns, for an enemy worth 'experienceValue'.
// Damage past its full health counts as its full health.
float earnedForDamage(float damage, float maxHealth, int experienceValue, const EarnRates &rates);

class Wallet
{
public:
	float points() const { return m_points; }
	int whole() const { return (int)m_points; }	///< what can be spent, and what is shown

	void earn(float amount, float maxPoints);
	bool canAfford(int cost) const { return cost <= whole(); }

	// Takes 'cost' when the wallet has it: a power is paid in full or not used.
	bool spend(int cost);

	// Takes up to 'cost', never going below zero: for a power fired without asking first (a script).
	void charge(int cost);

	void set(float points) { m_points = points; }

private:
	float m_points = 0.0f;
};

} // namespace CoH::GeneralsArsenal
