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

// A single weapon's own accuracy, set by name.
struct WeaponAccuracy
{
	AsciiString weapon;
	RangeProfile profile;
};

struct Config
{
	Rules rules;
	RangeProfile classProfiles[DAMAGE_NUM_TYPES];	///< by damage type; not set for types that never roll
	std::vector<WeaponAccuracy> weapons;			///< each Weapon line adds one
	std::vector<AsciiString> neverMisses;			///< each NeverMisses line adds its weapons
	float movingSpeed = 0.3f;						///< faster than this, in world units a frame, is moving
	AsciiString missFX;								///< played where a missed bullet lands; empty for none
};

const Config &config();
void parseConfigBlock(INI *ini);
void resetConfig();

// The profile the weapon rolls with; not set when it doesn't roll.
const RangeProfile &weaponProfile(const WeaponTemplate *weapon);

} // namespace CoH::Accuracy
