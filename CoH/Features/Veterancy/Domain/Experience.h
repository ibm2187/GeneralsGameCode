// CoH Veterancy: experience and how bonuses combine, as pure code. Experience comes from the damage a
// unit deals as well as its kills; a squad shares one pool of it.

#pragma once

#include <span>

namespace CoH::Veterancy
{

// The experience dealing 'damage' to a victim earns: the share 'damageShare' of the victim's kill
// value, for each of its full healths dealt. Dealing a whole health's worth to a fresh unit earns
// the share; the kill itself still pays the full value, as in the original game.
float experienceFromDamage(float damageShare, float damage, float victimMaxHealth, int killValue);

// Experience the engine counts in whole points. 'carry' keeps what isn't a whole point yet, so many
// small hits add up; returns the whole points to give now.
int wholePoints(float &carry, float gain);

// ---- A squad's shared pool ----------------------------------------------------------------------

// One member, as the last share left it and as it is now.
struct MemberExperience
{
	bool known = false;	///< it was in the squad at the last share; a new member isn't
	int seen = 0;		///< its experience after the last share
	int now = 0;		///< its experience now
};

// The squad's experience: the pool at the last share plus everything members earned since. A new
// member brings nothing and joins at the pool. With no known member (a new squad, a loaded game),
// the pool is the most any member has.
int squadPool(std::span<const MemberExperience> members);

// ---- Combining with the engine's bonuses ---------------------------------------------------------

// The engine adds its bonuses (rate of fire 1 + each bonus's share); 'total' has the engine's own
// rank bonus 'engineRank' in it. Swapping it for 'ours' scales the total, so it doesn't matter
// whether other subscribers (FireRate) scaled it before or after.
float swapRankBonus(float total, float engineRank, float ours);

// What a multiplier kept on the unit (sight) is scaled by when its rank's multiplier goes from 'from'
// to 'to'. Others' changes to the same value (a battle plan, an upgrade) stay.
float rankChange(float from, float to);

} // namespace CoH::Veterancy
