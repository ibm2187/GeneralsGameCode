// CoH Squads: see SquadBook.h.

#include "Features/Squads/Domain/SquadBook.h"

namespace CoH::Squads
{

Squad::Squad(SquadId id, const std::string &unit, int slots) :
	m_id(id),
	m_unit(unit),
	m_slots(slots > 0 ? slots : 1)
{
}

int Squad::add(MemberId member, float maxHealth)
{
	if (member == 0 || slotOf(member) >= 0)
		return -1;
	for (int slot = 0; slot < slotCount(); ++slot)
	{
		if (m_slots[slot].member == 0)
		{
			m_slots[slot].member = member;
			m_slots[slot].maxHealth = maxHealth;
			return slot;
		}
	}
	return -1;
}

bool Squad::remove(MemberId member)
{
	const int slot = slotOf(member);
	if (slot < 0)
		return false;
	m_slots[slot].member = 0;	// the slot keeps its max health: the bar shows what was lost
	return true;
}

int Squad::slotOf(MemberId member) const
{
	if (member == 0)
		return -1;
	for (int slot = 0; slot < slotCount(); ++slot)
	{
		if (m_slots[slot].member == member)
			return slot;
	}
	return -1;
}

MemberId Squad::memberIn(int slot) const
{
	return (slot >= 0 && slot < slotCount()) ? m_slots[slot].member : 0;
}

int Squad::memberCount() const
{
	int count = 0;
	for (const Slot &slot : m_slots)
	{
		if (slot.member != 0)
			++count;
	}
	return count;
}

MemberId Squad::leader() const
{
	for (const Slot &slot : m_slots)
	{
		if (slot.member != 0)
			return slot.member;
	}
	return 0;
}

std::vector<MemberId> Squad::members() const
{
	std::vector<MemberId> members;
	for (const Slot &slot : m_slots)
	{
		if (slot.member != 0)
			members.push_back(slot.member);
	}
	return members;
}

float Squad::fullHealth() const
{
	float total = 0.0f;
	float known = 0.0f;
	int unknown = 0;
	for (const Slot &slot : m_slots)
	{
		if (slot.maxHealth > 0.0f)
		{
			total += slot.maxHealth;
			known = slot.maxHealth;
		}
		else
			++unknown;
	}
	return total + unknown * known;
}

Squad &SquadBook::create(const std::string &unit, int slots)
{
	const SquadId id = m_nextId++;
	return m_squads.emplace(id, Squad(id, unit, slots)).first->second;
}

bool SquadBook::join(SquadId squad, MemberId member, float maxHealth)
{
	Squad *target = find(squad);
	if (target == nullptr || squadOf(member) != 0)
		return false;
	if (target->add(member, maxHealth) < 0)
		return false;
	m_memberSquads[member] = squad;
	return true;
}

SquadId SquadBook::leave(MemberId member)
{
	const auto found = m_memberSquads.find(member);
	if (found == m_memberSquads.end())
		return 0;
	const SquadId id = found->second;
	m_memberSquads.erase(found);

	Squad *squad = find(id);
	if (squad == nullptr)
		return 0;
	squad->remove(member);
	if (!squad->isEmpty())
		return 0;
	m_squads.erase(id);
	return id;
}

Squad *SquadBook::find(SquadId squad)
{
	const auto found = m_squads.find(squad);
	return found == m_squads.end() ? nullptr : &found->second;
}

const Squad *SquadBook::find(SquadId squad) const
{
	const auto found = m_squads.find(squad);
	return found == m_squads.end() ? nullptr : &found->second;
}

SquadId SquadBook::squadOf(MemberId member) const
{
	const auto found = m_memberSquads.find(member);
	return found == m_memberSquads.end() ? 0 : found->second;
}

void SquadBook::clear()
{
	m_squads.clear();
	m_memberSquads.clear();
	m_nextId = 1;
}

} // namespace CoH::Squads
