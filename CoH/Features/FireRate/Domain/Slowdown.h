// CoH FireRate: how much slower a weapon fires and how much harder each shot hits. A weapon that
// fires faster than its class allows is slowed to that limit, and each shot does more damage by the
// same factor, so its damage per second stays the same while every shot (and every hit roll) counts
// for more. Plain C++ with no engine headers, so it is unit tested.

#pragma once

namespace CoH::FireRate
{

// How a weapon fires, as its INI says, in logic frames.
struct FireCycle
{
	int minDelay = 0;		///< frames between shots (DelayBetweenShots; a range has a min and a max)
	int maxDelay = 0;
	int clipSize = 0;		///< shots before a reload; 0 is an endless clip
	int clipReload = 0;		///< frames the reload takes, in place of the delay after the clip's last shot
};

// Shots a second, averaged over a clip and its reload. 0 when the cycle takes no time (a weapon
// that fires once, or goes off where it stands): such a weapon has no rate to slow.
double shotsPerSecond(const FireCycle &cycle, double framesPerSecond);

// What the slice knows about one weapon.
struct WeaponFacts
{
	FireCycle cycle;
	double classSeconds = 0;	///< its damage type's Class line: at least this many seconds a shot; 0 is no line
	bool hasOwnLine = false;	///< a Weapon line names it...
	double ownSeconds = 0;		///< ...with this many seconds a shot; 0 never slows it
	double damageRadius = 0;	///< the larger of its primary and secondary blast
	double scatterRadius = 0;
	bool damageAtSelf = false;	///< goes off where the shooter stands
};

struct Rules
{
	double maxDamageRadius = 8;		///< a weapon with a bigger blast is an area weapon: never slowed by its class
	double maxSlowDown = 8;			///< a weapon is slowed at most this many times
	double damagePerSecond = 1;		///< the share of its damage per second a slowed weapon keeps
};

// What a weapon's bonuses are multiplied by.
struct Scaling
{
	double slowDown = 1;	///< delays and reloads are this many times longer
	double damage = 1;		///< each shot does this many times the damage

	bool changes() const { return slowDown != 1 || damage != 1; }
};

// The seconds a shot the weapon is held to, 0 when it isn't held to any: its own line, else its
// class's line unless it is an area weapon, scatters, or goes off where it stands.
double secondsPerShotLimit(const WeaponFacts &facts, const Rules &rules);

Scaling scalingFor(const WeaponFacts &facts, const Rules &rules, double framesPerSecond);

} // namespace CoH::FireRate
