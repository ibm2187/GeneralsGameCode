// CoH Veterancy: see Experience.h.

#include "Features/Veterancy/Domain/Experience.h"

#include <algorithm>

namespace CoH::Veterancy
{

float experienceFromDamage(float damageShare, float damage, float victimMaxHealth, int killValue)
{
	if (damageShare <= 0.0f || damage <= 0.0f || victimMaxHealth <= 0.0f || killValue <= 0)
		return 0.0f;
	const float healths = std::min(damage / victimMaxHealth, 1.0f);
	return damageShare * static_cast<float>(killValue) * healths;
}

int wholePoints(float &carry, float gain)
{
	if (gain <= 0.0f)
		return 0;
	carry += gain;
	const int whole = static_cast<int>(carry);
	carry -= static_cast<float>(whole);
	return whole;
}

int squadPool(std::span<const MemberExperience> members)
{
	int pool = 0;
	bool anyKnown = false;
	for (const MemberExperience &member : members)
	{
		if (member.known)
		{
			pool = anyKnown ? std::max(pool, member.seen) : member.seen;
			anyKnown = true;
		}
	}
	if (!anyKnown)
	{
		for (const MemberExperience &member : members)
			pool = std::max(pool, member.now);
		return pool;
	}
	for (const MemberExperience &member : members)
	{
		if (member.known)
			pool += member.now - member.seen;
	}
	return std::max(pool, 0);
}

float swapRankBonus(float total, float engineRank, float ours)
{
	if (engineRank <= 0.0f)
		return total;
	return total * ours / engineRank;
}

float rankChange(float from, float to)
{
	if (from <= 0.0f || to <= 0.0f)
		return 1.0f;
	return to / from;
}

} // namespace CoH::Veterancy
