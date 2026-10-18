// CoH FireRate: the tuning, read from the CoHFireRate block of the pack (Data/INI/10_FireRate.ini),
// and each weapon's slowdown, worked out once per game. Back to these defaults after every game.

#pragma once

#include "Common/AsciiString.h"
#include "GameLogic/Damage.h"

#include "Features/FireRate/Domain/Slowdown.h"

#include <vector>

class INI;
class WeaponTemplate;

namespace CoH::FireRate
{

// A Weapon line: one weapon's own limit, over its class's.
struct OwnLimit
{
	AsciiString weapon;
	Real seconds = 0;	///< 0 never slows it
};

struct Config
{
	Real classSeconds[DAMAGE_NUM_TYPES] = {};	///< by DamageType; 0 is no Class line
	std::vector<OwnLimit> weapons;				///< each Weapon line adds one

	Real maxDamageRadius = 8;
	Real maxSlowDown = 8;
	Real damagePerSecond = 1;
};

const Config &config();
void parseConfigBlock(INI *ini);
void resetConfig();

// How much slower 'weapon' fires and how much harder it hits in this game.
const Scaling &weaponScaling(const WeaponTemplate *weapon);

} // namespace CoH::FireRate
