// CoH Veterancy: what each rank gives a unit, as pure code. The bonuses come by class (infantry,
// vehicles, aircraft), and a unit's own lines change single bonuses of one rank.

#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

namespace CoH::Veterancy
{

// The engine's ranks (VeterancyLevel), in the same order.
enum class Rank { Regular, Veteran, Elite, Heroic };
inline constexpr std::size_t RankCount = 4;

constexpr Rank rankOf(int level)
{
	if (level <= 0)
		return Rank::Regular;
	if (level >= static_cast<int>(RankCount) - 1)
		return Rank::Heroic;
	return static_cast<Rank>(level);
}

// Everything one rank changes. Multipliers are 1 and resistance 0 for no change.
struct RankBonus
{
	float accuracy = 1.0f;				///< hit chance (Accuracy's "Veterancy" modifier)
	float damageTaken = 1.0f;			///< damage the unit takes, after its armor
	float suppressionResistance = 0.0f;	///< share of suppression it shrugs off, 0..1 (Suppression)
	float speed = 1.0f;					///< its top speed
	float sight = 1.0f;					///< how far it sees and clears the shroud
	float reload = 1.0f;				///< its rate of fire and reloads, in place of the engine's own for the rank
	std::string upgrade;				///< an object upgrade it is given on reaching the rank (a drone, an ability)
};

// One line of a unit's own: only the bonuses it names change.
struct BonusChange
{
	std::optional<float> accuracy;
	std::optional<float> damageTaken;
	std::optional<float> suppressionResistance;
	std::optional<float> speed;
	std::optional<float> sight;
	std::optional<float> reload;
	std::optional<std::string> upgrade;

	void applyTo(RankBonus &bonus) const;
};

enum class UnitClass { Infantry, Vehicle, Aircraft, Other };
inline constexpr std::size_t ClassCount = 3;	///< the classes with tables: all but Other

using RankTable = std::array<RankBonus, RankCount>;	///< by Rank; Regular is usually all ones

struct ObjectLine
{
	std::string name;
	Rank rank = Rank::Veteran;
	BonusChange change;
};

struct Rules
{
	std::array<RankTable, ClassCount> classes {};	///< by UnitClass
	std::vector<ObjectLine> objects;				///< a unit's own lines, in the order read
};

// What the engine knows about a unit, as plain facts. A helicopter is an aircraft.
struct Kinds
{
	bool aircraft = false;
	bool vehicle = false;
	bool infantry = false;
};

constexpr UnitClass classify(const Kinds &kinds)
{
	if (kinds.aircraft)
		return UnitClass::Aircraft;
	if (kinds.vehicle)
		return UnitClass::Vehicle;
	if (kinds.infantry)
		return UnitClass::Infantry;
	return UnitClass::Other;
}

// A general's version of a unit without the general: "Tank_ChinaTankOverlord" is "ChinaTankOverlord".
std::string_view baseName(std::string_view name);

// The unit's ranks: its class's table, changed by the lines of its plain unit and then by its own
// lines ("Tank_ChinaTankOverlord" takes the ChinaTankOverlord lines, then its own). Names match
// without case. A unit of no class has no bonuses but its own lines.
RankTable tableFor(const Rules &rules, std::string_view name, UnitClass unitClass);

// ---- The tooltip --------------------------------------------------------------------------------

enum class Stat { Accuracy, DamageTaken, SuppressionResistance, Speed, Sight, Reload };

// One bonus as the player reads it: "+20% accuracy" is { Accuracy, 20 }, "-15% damage taken" is
// { DamageTaken, -15 }, "40% suppression resistance" is { SuppressionResistance, 40 }.
struct StatChange
{
	Stat stat;
	int percent;
};

// The bonuses of a rank that change something, in Stat order, rounded to whole percents.
std::vector<StatChange> changesOf(const RankBonus &bonus);

} // namespace CoH::Veterancy
