// CoH Squads: what other slices may ask about squads, and do with them. The only header of this slice
// others include.
//
// Infantry the pack lists is made as a squad: one order makes several soldiers that are selected,
// ordered, carried and counted as one unit. Each member is an ordinary object; the squad is a list of
// them with one slot per member it was made with. A lost member leaves its slot empty, which is what
// Reinforce fills. Everything here answers "no squad" while the Squads flag is off.

#pragma once

#include <vector>

class Object;
class ThingTemplate;
struct Coord3D;

namespace CoH::Squads
{

// The game being played makes squads.
bool isActive();

// ---- Who is in which squad ----------------------------------------------------------------------

int squadOf(const Object *object);			///< the object's squad; 0 when it is in none
std::vector<Object *> members(int squad);	///< the living members, in slot order
Object *leader(int squad);					///< the first living member, who speaks for the squad

// Whether the object counts as a unit of its own: true for anything not in a squad and for a squad's
// leader; false for the other members. Upkeep's population counts these.
bool countsAsUnit(const Object *object);

// ---- What a squad lost, and filling it again (Reinforce, Weapon pickup) -------------------------

int slotCount(int squad);		///< the members it was made with
int missingMembers(int squad);	///< empty slots

// The kind of unit slot 'slot' was made with; null past the last slot or for no squad.
const ThingTemplate *unitOfSlot(int squad, int slot);
std::vector<int> emptySlots(int squad);

// Puts 'member' (made by the caller, for the squad's player) in the squad's first empty slot.
// False when the squad is full or gone, or the object is in a squad already.
bool addMember(int squad, Object *member);

// ---- Orders (Retreat, Abilities) ----------------------------------------------------------------

// Sends the squad to 'goal' in formation, as the AI would order it.
void moveTo(int squad, const Coord3D &goal);

// Where the member's formation slot put it, the last time its squad was sent somewhere. False when
// it isn't in a squad or hasn't been sent anywhere yet. Cover moves members from here to cover.
bool formationSlot(const Object *member, Coord3D *position);

// ---- What a squad costs (Upkeep) ----------------------------------------------------------------

bool isSquadUnit(const ThingTemplate *unit);	///< ordering this unit makes a squad

// What one order of this unit costs before the player's own modifiers: the whole squad's price when
// it makes a squad, else the unit's own build cost.
int buildCostOf(const ThingTemplate *unit);

} // namespace CoH::Squads
