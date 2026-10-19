// CoH DirectionalArmor: see Names.h.

#include "Features/DirectionalArmor/Domain/Names.h"

#include <algorithm>

namespace CoH::DirectionalArmor
{

namespace
{

bool sameIgnoringCase(std::string_view a, std::string_view b)
{
	const auto lower = [](char c) { return (c >= 'A' && c <= 'Z') ? char(c - 'A' + 'a') : c; };
	return a.size() == b.size() && std::ranges::equal(a, b, {}, lower, lower);
}

} // namespace

int nameMatch(std::string_view entry, std::string_view name)
{
	if (entry.empty())
		return 0;
	if (sameIgnoringCase(entry, name))
		return 2;
	if (name.size() > entry.size() + 1)
	{
		const std::string_view tail = name.substr(name.size() - entry.size());
		if (name[name.size() - entry.size() - 1] == '_' && sameIgnoringCase(entry, tail))
			return 1;
	}
	return 0;
}

bool namesCover(std::string_view entry, std::string_view name)
{
	return nameMatch(entry, name) > 0;
}

} // namespace CoH::DirectionalArmor
