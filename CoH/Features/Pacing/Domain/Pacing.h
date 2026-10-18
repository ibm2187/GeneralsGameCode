// CoH Pacing: the rules, as pure code. What a unit costs and how long it takes, by its class or its
// own line, and the skirmish AI's handicap by difficulty.

#pragma once

#include <array>
#include <cstddef>
#include <string>
#include <string_view>
#include <vector>

namespace CoH::Pacing
{

enum class UnitClass { Infantry, Vehicle, Aircraft, Structure, Unchanged };
inline constexpr std::size_t ClassCount = 4;	///< the classes with multipliers: all but Unchanged

// What the engine knows about a thing, as plain facts.
struct Kinds
{
	bool exempt = false;	///< of an exempt kind (dozers, workers, harvesters, drones)
	bool structure = false;
	bool aircraft = false;
	bool vehicle = false;
	bool infantry = false;
};

// Exempt things stay unchanged; a helicopter is both a vehicle and an aircraft and counts as an aircraft.
constexpr UnitClass classify(const Kinds &kinds)
{
	if (kinds.exempt)
		return UnitClass::Unchanged;
	if (kinds.structure)
		return UnitClass::Structure;
	if (kinds.aircraft)
		return UnitClass::Aircraft;
	if (kinds.vehicle)
		return UnitClass::Vehicle;
	if (kinds.infantry)
		return UnitClass::Infantry;
	return UnitClass::Unchanged;
}

struct Scale
{
	float cost = 1.0f;
	float buildTime = 1.0f;
};

struct ObjectScale
{
	std::string name;
	Scale scale;
};

struct Rules
{
	std::array<Scale, ClassCount> classes {};	///< by UnitClass
	std::vector<ObjectScale> objects;			///< own lines, which win over the class
	int costStep = 5;							///< costs are rounded to this many dollars
};

// A general's version of a unit without the general: "Tank_ChinaTankOverlord" is "ChinaTankOverlord".
std::string_view baseName(std::string_view name);

// The object's own line, else its plain unit's line, else its class's. Names match without case.
Scale scaleFor(const Rules &rules, std::string_view name, UnitClass unitClass);

// 'amount' times 'multiplier', to the nearest 'step'. A positive amount stays at least one step.
int scaled(int amount, float multiplier, int step = 1);

enum class Difficulty { Easy, Medium, Hard };

// How the skirmish AI of one difficulty is held back (or helped): its income and its build times.
struct Handicap
{
	float income = 1.0f;
	float buildTime = 1.0f;
};

using Handicaps = std::array<Handicap, 3>;	///< by Difficulty

// Income times the multiplier, rounded down, as the engine pays whole dollars.
unsigned int scaledIncome(unsigned int amount, float multiplier);

} // namespace CoH::Pacing
