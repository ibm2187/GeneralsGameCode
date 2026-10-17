// CoH Accuracy: the named hit modifiers, applied to a base chance in a defined order. Plain C++,
// unit tested.
//
// Each modifier belongs to a feature and counts only while that feature is on. A modifier added
// under a name already in the stack stands in for the one there while its own feature is on, so a
// slice can replace a built-in one (Veterancy's own "Veterancy") without editing Accuracy, and the
// built-in one comes back when that slice is off. Built-in modifiers always give way to added ones,
// whichever was added first; between two added ones, the later one wins.

#pragma once

#include "Features/Accuracy/Domain/Shot.h"

#include <string>
#include <vector>

namespace CoH
{
class Feature;
}

namespace CoH::Accuracy
{

class ModifierStack
{
public:
	void add(const Feature &owner, const std::string &name, int order, ModifierFunction modifier);
	void addBuiltIn(const Feature &owner, const std::string &name, int order, ModifierFunction modifier);

	// The base chance with every counting modifier applied, in order. Not clamped.
	float apply(float baseChance, const Shot &shot) const;

	// The names of the modifiers that count now, in the order they apply.
	std::vector<std::string> activeNames() const;

private:
	struct Entry
	{
		std::string name;
		int order;
		int added;			///< which add call made it
		bool builtIn;
		const Feature *owner;
		ModifierFunction modifier;
	};

	void insert(const Entry &entry);
	bool counts(const Entry &entry) const;

	std::vector<Entry> m_entries;	///< sorted by order, then name, then when added
};

} // namespace CoH::Accuracy
