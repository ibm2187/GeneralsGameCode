// CoH DirectionalArmor: the tuning, read from the CoHDirectionalArmor block of the pack
// (Data/INI/10_DirectionalArmor.ini), and each object's armor and each weapon's penetration, looked up
// once per game. Back to these defaults after every game.

#pragma once

#include "Common/AsciiString.h"
#include "Common/KindOf.h"
#include "GameLogic/Damage.h"

#include "Features/DirectionalArmor/Domain/Armor.h"
#include "Features/DirectionalArmor/Domain/Blast.h"
#include "Features/DirectionalArmor/Domain/Flank.h"

#include <optional>
#include <vector>

class INI;
class Object;
class WeaponTemplate;

namespace CoH::DirectionalArmor
{

// A Unit line, or a Weapon line.
template <class Values>
struct NamedLine
{
	AsciiString name;
	Values values;
};

// A Kind line: the armor of vehicles of this kind with no Unit line of their own.
struct KindLine
{
	KindOfType kind = KINDOF_INVALID;
	ArmorValues armor;
};

struct Config
{
	Real deflectedDamage = 0.1f;
	SideDamage sideDamage;		///< how hard a hit on each side lands
	Real frontArc = 90.0f;		///< full width, degrees
	Real rearArc = 90.0f;
	Real areaRadius = 20.0f;	///< weapons with a bigger damage radius never roll
	AsciiString deflectFX;
	Real deflectTextPenetration = 0.0f;	///< "Deflected" floats for weapons with at least this; 0 never

	std::optional<Penetration> byDamageType[DAMAGE_NUM_TYPES];	///< Penetration lines
	std::vector<NamedLine<Penetration>> weapons;				///< Weapon lines
	std::vector<KindLine> kinds;								///< Kind lines, first match wins
	std::vector<NamedLine<ArmorValues>> units;					///< Unit lines
	KindOfMaskType exempt;										///< no Kind armor for these

	Real infantryDamage[DAMAGE_NUM_TYPES];		///< InfantryDamage lines; 1 where there is none
	KindOfMaskType infantryDamageFrom;			///< attackers of these kinds get it
	KindOfMaskType infantryDamageNotFrom;		///< attackers of these kinds don't
	BlastFalloff infantryBlast;					///< the splash against infantry, near to far

	bool flanking = true;		///< the skirmish AI flanks
	FlankRules flank;
	UnsignedInt flankCheckFrames = 30;
	UnsignedInt flankTimeoutFrames = 240;
	UnsignedInt flankRestFrames = 360;

	Arcs arcs;					///< from frontArc and rearArc, once read

	Config();
};

const Config &config();
void parseConfigBlock(INI *ini);
void resetConfig();

// The object's armor in this game, or nothing when it has none (infantry, buildings, aircraft, any
// object no Unit or Kind line covers).
const ArmorValues *armorOf(const Object &object);

// The weapon's penetration in this game, or nothing when it never rolls (no Weapon line, no
// Penetration line for its DamageType).
const Penetration *penetrationOf(const WeaponTemplate &weapon);

// How much more (or less) damage the weapon's DamageType does to infantry from a vehicle; 1 for none.
Real infantryDamageFactor(DamageType type);

} // namespace CoH::DirectionalArmor
