// CoH Squads: which units are made as squads, and what a squad costs. The pack lists each squad by
// the unit the player orders; a general's version of that unit ("AirF_AmericaInfantryRanger") is made
// the same way unless the pack lists it too.

#pragma once

#include <string>
#include <vector>

namespace CoH::Squads
{

// Members of another kind that come with the ordered unit, such as a hero's escort.
struct Escort
{
	std::string unit;
	int count = 0;
};

struct SquadType
{
	std::string unit;		///< the unit the player orders; the squad's first members are this kind
	int size = 1;			///< how many of 'unit' the squad has
	std::vector<Escort> escorts;

	int memberCount() const;	///< every member, escorts included
	// The kind of unit slot 'slot' holds: 'unit' for the first 'size' slots, then the escorts in order.
	// Empty past the last slot.
	std::string unitOfSlot(int slot) const;
};

class SquadTypes
{
public:
	void add(const SquadType &type);	///< a later entry for the same unit replaces the earlier one
	void clear() { m_types.clear(); }
	bool empty() const { return m_types.empty(); }

	// The squad made when 'unit' is ordered: its own entry, or else the entry for its name without
	// a general's prefix ("AirF_", "GC_Chem_"). Null when it is made alone.
	const SquadType *find(const std::string &unit) const;

private:
	const SquadType *findExact(const std::string &unit) const;

	std::vector<SquadType> m_types;
};

// What a whole squad costs (or takes to make), from what one order of its unit costs. A factory may
// make several units per order ('orderQuantity', Red Guards: 2), and the squad holds 'size' of them;
// 'escortAmount' is the escorts' own cost added together. 'percent' scales the sum: below 100 is a
// discount for buying a squad. Rounded to the nearest whole number.
int squadAmount(int orderAmount, int orderQuantity, int size, int escortAmount, int percent);

} // namespace CoH::Squads
