// CoH VehicleMovement: see Names.h.

#include "Features/VehicleMovement/Domain/Names.h"

#include <algorithm>

namespace CoH::VehicleMovement
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

bool startsWith(std::string_view entry, std::string_view name)
{
	return !entry.empty() && name.size() >= entry.size() && sameIgnoringCase(entry, name.substr(0, entry.size()));
}

} // namespace CoH::VehicleMovement
