// CoH Accuracy: what a hit modifier is told about a shot, and what it answers. Plain C++ with no
// engine headers; the engine objects are there for modifiers of other slices to look at.

#pragma once

class Object;
class WeaponTemplate;

namespace CoH::Accuracy
{

enum class TargetSize
{
	Infantry,
	Vehicle,
	Aircraft,
	Building,
	Count
};

// One shot at an object, as the engine saw it when the weapon fired.
struct Shot
{
	const Object *shooter = nullptr;
	const Object *target = nullptr;
	const WeaponTemplate *weapon = nullptr;

	float distance = 0.0f;		///< from the shooter to the target, measured as the weapon's range is
	float range = 0.0f;			///< the weapon's attack range, its bonuses included
	bool shooterMoving = false;	///< a soldier riding in a vehicle moves with it
	bool targetMoving = false;
	TargetSize targetSize = TargetSize::Vehicle;
	int veterancy = 0;			///< the shooter's rank: 0 regular, 1 veteran, 2 elite, 3 heroic
};

// What one modifier does to the hit chance: chance = chance * multiply + add. Chances are 0..1, so
// add = -0.1f takes ten points off.
struct HitModifier
{
	float multiply = 1.0f;
	float add = 0.0f;
};

inline HitModifier scaleChance(float multiply) { return { multiply, 0.0f }; }
inline HitModifier addToChance(float add) { return { 1.0f, add }; }

using ModifierFunction = HitModifier (*)(const Shot &shot);

// Where a modifier goes in the stack: lower orders apply first; modifiers of the same order apply
// in the order of their names. Scaling commutes, so the order only matters for modifiers that add.
namespace ModifierOrder
{
constexpr int Range = 100;
constexpr int Movement = 200;
constexpr int TargetSize = 300;
constexpr int Cover = 400;
constexpr int Suppression = 500;
constexpr int Veterancy = 600;
}

} // namespace CoH::Accuracy
