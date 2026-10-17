// CoH Accuracy: the tuning, read from the CoHAccuracy block of the pack (Data/INI/10_Accuracy.ini).
// Back to these defaults after every game.

#pragma once

#include "Common/AsciiString.h"
#include "GameLogic/Damage.h"

#include "Features/Accuracy/Domain/HitChance.h"

#include <vector>

class INI;
class WeaponTemplate;

namespace CoH::Accuracy
{

// A single weapon's own base accuracy, set by name.
struct WeaponAccuracy
{
	AsciiString weapon;
	int percent = 0;
};

struct Config
{
	Rules rules;
	int classPercent[DAMAGE_NUM_TYPES];		///< by damage type; -1 for types that never roll
	std::vector<WeaponAccuracy> weapons;	///< each Weapon line adds one
	float movingSpeed = 0.3f;				///< faster than this, in world units a frame, is moving
	AsciiString missFX;						///< played where a missed bullet lands; empty for none

	Config();
};

const Config &config();
void parseConfigBlock(INI *ini);
void resetConfig();

// The base accuracy of the weapon (0..1), or a negative value when it doesn't roll.
float weaponBaseAccuracy(const WeaponTemplate *weapon);

} // namespace CoH::Accuracy
