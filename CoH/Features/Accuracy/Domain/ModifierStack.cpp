// CoH Accuracy: see ModifierStack.h.

#include "Features/Accuracy/Domain/ModifierStack.h"

#include "Kernel/Domain/Feature.h"

#include <algorithm>
#include <tuple>

namespace CoH::Accuracy
{

void ModifierStack::add(const Feature &owner, const std::string &name, int order, ModifierFunction modifier)
{
	insert({ name, order, (int)m_entries.size(), false, &owner, modifier });
}

void ModifierStack::addBuiltIn(const Feature &owner, const std::string &name, int order, ModifierFunction modifier)
{
	insert({ name, order, (int)m_entries.size(), true, &owner, modifier });
}

void ModifierStack::insert(const Entry &entry)
{
	if (entry.modifier == nullptr || entry.name.empty())
		return;

	const auto comesBefore = [](const Entry &a, const Entry &b)
	{
		return std::tie(a.order, a.name, a.added) < std::tie(b.order, b.name, b.added);
	};
	m_entries.insert(std::upper_bound(m_entries.begin(), m_entries.end(), entry, comesBefore), entry);
}

namespace
{

template <typename Entry>
bool standsInFor(const Entry &other, const Entry &entry)
{
	if (other.builtIn != entry.builtIn)
		return entry.builtIn;
	return other.added > entry.added;
}

} // namespace

// On, and no other modifier of the same name that is on stands in for it.
bool ModifierStack::counts(const Entry &entry) const
{
	if (!entry.owner->isActive())
		return false;
	for (const Entry &other : m_entries)
	{
		if (&other != &entry && other.name == entry.name && other.owner->isActive() && standsInFor(other, entry))
			return false;
	}
	return true;
}

float ModifierStack::apply(float baseChance, const Shot &shot) const
{
	float chance = baseChance;
	for (const Entry &entry : m_entries)
	{
		if (!counts(entry))
			continue;
		const HitModifier modifier = entry.modifier(shot);
		chance = chance * modifier.multiply + modifier.add;
	}
	return chance;
}

std::vector<std::string> ModifierStack::activeNames() const
{
	std::vector<std::string> names;
	for (const Entry &entry : m_entries)
	{
		if (counts(entry))
			names.push_back(entry.name);
	}
	return names;
}

} // namespace CoH::Accuracy
