// CoH FireRate: see Slowdown.h.

#include "Features/FireRate/Domain/Slowdown.h"

#include <algorithm>

namespace CoH::FireRate
{

double shotsPerSecond(const FireCycle &cycle, double framesPerSecond)
{
	const double delay = (cycle.minDelay + cycle.maxDelay) / 2.0;
	const int shots = cycle.clipSize > 0 ? cycle.clipSize : 1;
	// After the clip's last shot the weapon waits for its reload instead of the delay.
	const double frames = cycle.clipSize > 0 ? (shots - 1) * delay + cycle.clipReload : delay;
	if (frames <= 0 || framesPerSecond <= 0)
		return 0;
	return shots * framesPerSecond / frames;
}

double topShotsPerSecond(const WeaponFacts &facts, double framesPerSecond)
{
	return shotsPerSecond(facts.cycle, framesPerSecond) * (facts.spinUp > 1 ? facts.spinUp : 1);
}

double secondsPerShotLimit(const WeaponFacts &facts, const Rules &rules)
{
	if (facts.damage < rules.minDamage)
		return 0;
	if (facts.hasOwnLine)
		return facts.ownSeconds;
	if (facts.damageRadius > rules.maxDamageRadius || facts.scatterRadius > 0 || facts.damageAtSelf)
		return 0;
	return facts.classSeconds;
}

Scaling scalingFor(const WeaponFacts &facts, const Rules &rules, double framesPerSecond)
{
	const double limit = secondsPerShotLimit(facts, rules);
	const double rate = topShotsPerSecond(facts, framesPerSecond);
	if (limit <= 0 || rate <= 0)
		return {};

	const double slowDown = std::min(rate * limit, rules.maxSlowDown);
	if (slowDown <= 1)
		return {};

	Scaling scaling;
	scaling.slowDown = slowDown;
	// A shot never does less than it did: a weapon only gets slower and harder hitting.
	scaling.damage = std::max(1.0, slowDown * rules.damagePerSecond);
	return scaling;
}

} // namespace CoH::FireRate
