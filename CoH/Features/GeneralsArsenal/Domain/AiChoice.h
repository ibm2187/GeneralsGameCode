// CoH GeneralsArsenal: which General's power the skirmish AI uses next. The AI thinks every few
// seconds and uses at most one power each time: a super weapon on an enemy base first, then a strike,
// then air cover over its army when it is fighting. Once it may use a super weapon it saves toward
// it, spending on smaller powers only what it can spare.

#pragma once

#include "PowerRules.h"

#include <vector>

namespace CoH::GeneralsArsenal
{

struct AiCandidate
{
	AiUse use = AiUse::None;
	int cost = 0;
	bool allowed = false;	///< rank, airfield, points: what the power's button would allow
	bool ready = false;		///< its cooldown is over
	bool unlocked = false;	///< the rank allows it, whatever else is missing
	bool hasTarget = false;	///< the AI found something worth using it on
};

struct AiSituation
{
	int points = 0;
};

struct AiPolicy
{
	float superweaponSavingShare = 0.3f;	///< points kept back, as a share of an unlocked super weapon's cost
};

// The index in 'candidates' of the power to use now, or -1 for none.
int chooseAiPower(const std::vector<AiCandidate> &candidates, const AiSituation &situation, const AiPolicy &policy);

} // namespace CoH::GeneralsArsenal
