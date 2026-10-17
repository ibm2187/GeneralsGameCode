// CoH Squads: a squad takes the room of one unit in a transport or a garrison. Its first member in
// takes the room the unit kind takes; the members after it ride along.

#pragma once

#include "Features/Squads/Domain/SquadBook.h"

#include <vector>

namespace CoH::Squads
{

// One object inside a container: its squad (0 for none) and the room it takes on its own.
struct Rider
{
	SquadId squad = 0;
	int room = 1;
};

// The room the riders take, each squad counted once, by its first rider.
int roomUsed(const std::vector<Rider> &inside);

// The room 'rider' needs to get in: none when a member of its squad is already inside.
int roomNeeded(const Rider &rider, const std::vector<Rider> &inside);

} // namespace CoH::Squads
