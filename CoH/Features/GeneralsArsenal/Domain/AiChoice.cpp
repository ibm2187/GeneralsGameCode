// CoH GeneralsArsenal: see AiChoice.h.

#include "AiChoice.h"

#include <algorithm>
#include <cmath>

namespace CoH::GeneralsArsenal
{

namespace
{

// The points the AI keeps for the dearest super weapon its rank allows.
int savingFor(const std::vector<AiCandidate> &candidates, const AiPolicy &policy)
{
	int dearest = 0;
	for (const AiCandidate &candidate : candidates)
	{
		if (candidate.use == AiUse::Superweapon && candidate.unlocked)
			dearest = std::max(dearest, candidate.cost);
	}
	return (int)std::ceil(dearest * policy.superweaponSavingShare);
}

int firstUsable(const std::vector<AiCandidate> &candidates, const AiSituation &situation, AiUse use, int spare)
{
	for (size_t i = 0; i < candidates.size(); ++i)
	{
		const AiCandidate &candidate = candidates[i];
		if (candidate.use != use || !candidate.allowed || !candidate.ready || !candidate.hasTarget)
			continue;
		if (use != AiUse::Superweapon && situation.points - candidate.cost < spare)
			continue;
		return (int)i;
	}
	return -1;
}

} // namespace

int chooseAiPower(const std::vector<AiCandidate> &candidates, const AiSituation &situation, const AiPolicy &policy)
{
	const int spare = savingFor(candidates, policy);
	for (AiUse use : { AiUse::Superweapon, AiUse::Strike, AiUse::Cover })
	{
		const int chosen = firstUsable(candidates, situation, use, spare);
		if (chosen >= 0)
			return chosen;
	}
	return -1;
}

} // namespace CoH::GeneralsArsenal
