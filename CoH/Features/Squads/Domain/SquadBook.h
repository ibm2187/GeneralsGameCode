// CoH Squads: who is in which squad. Each squad has a fixed number of slots, one per member it is
// made with. A member that dies leaves its slot empty, so the squad knows what it lost and Reinforce
// (later) can fill the slot again. A squad with no one left is gone.

#pragma once

#include <functional>
#include <map>
#include <string>
#include <vector>

namespace CoH::Squads
{

using MemberId = unsigned int;	///< the engine's ObjectID; 0 is no one
using SquadId = int;			///< 0 is no squad

class Squad
{
public:
	Squad(SquadId id, const std::string &unit, int slots);

	SquadId id() const { return m_id; }
	const std::string &unit() const { return m_unit; }	///< the unit that was ordered
	int slotCount() const { return (int)m_slots.size(); }

	// Puts 'member' in the first empty slot; returns the slot, or -1 when every slot is taken.
	// 'maxHealth' is the member's, for the squad's health bar.
	int add(MemberId member, float maxHealth);
	bool remove(MemberId member);	///< empties the member's slot; false when it isn't in this squad

	int slotOf(MemberId member) const;	///< -1 when it isn't in this squad
	MemberId memberIn(int slot) const;	///< 0 when the slot is empty
	int memberCount() const;			///< members alive
	int missingCount() const { return slotCount() - memberCount(); }
	bool isEmpty() const { return memberCount() == 0; }

	MemberId leader() const;	///< the first member in slot order: speaks for the squad
	std::vector<MemberId> members() const;	///< in slot order

	// The first member in slot order that is among 'others' (the riders of one container): speaks
	// for the members there. 0 when none of them is.
	MemberId leaderAmong(const std::vector<MemberId> &others) const;

	// The squad's whole health when full: every slot's max health. An empty slot that never had a
	// member counts as its neighbours do.
	float fullHealth() const;

private:
	friend class SquadBook;

	struct Slot
	{
		MemberId member = 0;
		float maxHealth = 0.0f;
	};

	SquadId m_id;
	std::string m_unit;
	std::vector<Slot> m_slots;
};

// What a saved game keeps of a book: every squad with all its slots, the empty ones too (they keep
// the max health of the member that was lost), and the id the next squad gets.
struct SavedSlot
{
	MemberId member = 0;
	float maxHealth = 0.0f;
};

struct SavedSquad
{
	SquadId id = 0;
	std::string unit;
	std::vector<SavedSlot> slots;
};

struct SavedBook
{
	SquadId nextId = 1;
	std::vector<SavedSquad> squads;
};

class SquadBook
{
public:
	Squad &create(const std::string &unit, int slots);	///< a new squad, with no members yet

	// Adds 'member' to 'squad'; false when the squad is full or gone, or the member is in a squad.
	bool join(SquadId squad, MemberId member, float maxHealth);

	// Takes 'member' out of its squad (it died or left). A squad left with no one is gone; returns
	// that squad's id then, 0 otherwise.
	SquadId leave(MemberId member);

	Squad *find(SquadId squad);
	const Squad *find(SquadId squad) const;
	SquadId squadOf(MemberId member) const;	///< 0 when none

	const std::map<SquadId, Squad> &squads() const { return m_squads; }	///< in the order they were made
	void clear();

	SavedBook save() const;

	// Replaces the book with a saved one. A member 'exists' says no to (it is gone from the game) leaves
	// its slot empty, and so does a member listed twice after its first slot; a squad left with no
	// one is dropped. Returns the members put back.
	int restore(const SavedBook &saved, const std::function<bool(MemberId)> &exists);

private:
	SquadId m_nextId = 1;
	std::map<SquadId, Squad> m_squads;
	std::map<MemberId, SquadId> m_memberSquads;
};

} // namespace CoH::Squads
