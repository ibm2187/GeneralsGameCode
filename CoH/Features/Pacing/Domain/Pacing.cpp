// CoH Pacing: see Pacing.h.

#include "Features/Pacing/Domain/Pacing.h"

#include <algorithm>
#include <ranges>

namespace CoH::Pacing
{

namespace
{

constexpr char lower(char c)
{
	return (c >= 'A' && c <= 'Z') ? char(c - 'A' + 'a') : c;
}

bool sameName(std::string_view a, std::string_view b)
{
	return std::ranges::equal(a, b, [](char x, char y) { return lower(x) == lower(y); });
}

const ObjectScale *ownLine(const Rules &rules, std::string_view name)
{
	const auto found = std::ranges::find_if(rules.objects, [&](const ObjectScale &line) { return sameName(line.name, name); });
	return found == rules.objects.end() ? nullptr : &*found;
}

} // namespace

std::string_view baseName(std::string_view name)
{
	const std::size_t underscore = name.find('_');
	if (underscore == std::string_view::npos || underscore + 1 >= name.size())
		return name;
	return name.substr(underscore + 1);
}

Scale scaleFor(const Rules &rules, std::string_view name, UnitClass unitClass)
{
	if (const ObjectScale *own = ownLine(rules, name))
		return own->scale;
	if (const ObjectScale *plain = ownLine(rules, baseName(name)))
		return plain->scale;
	if (unitClass == UnitClass::Unchanged)
		return {};
	return rules.classes[static_cast<std::size_t>(unitClass)];
}

int scaled(int amount, float multiplier, int step)
{
	if (amount <= 0 || multiplier == 1.0f)
		return amount;
	step = std::max(step, 1);
	const int steps = static_cast<int>(static_cast<float>(amount) * multiplier / static_cast<float>(step) + 0.5f);
	return std::max(steps, 1) * step;
}

unsigned int scaledIncome(unsigned int amount, float multiplier)
{
	if (multiplier == 1.0f)
		return amount;
	if (multiplier <= 0.0f)
		return 0;
	return static_cast<unsigned int>(static_cast<float>(amount) * multiplier);
}

} // namespace CoH::Pacing
