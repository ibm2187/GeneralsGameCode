// CoH DirectionalArmor: which INI line names an object or a weapon, as pure code. Unit tested in
// Tests/ArmorTest.cpp.

#pragma once

#include <string_view>

namespace CoH::DirectionalArmor
{

// A Unit or Weapon line names 'entry'. It covers that name and the generals' versions of it, which
// put a prefix and an underscore before it: "OverlordTankGun" covers "Nuke_OverlordTankGun" and
// "ChinaTankOverlord" covers "Tank_ChinaTankOverlord". Case doesn't matter.
bool namesCover(std::string_view entry, std::string_view name);

// How well: 2 for the name itself, 1 for a general's version, 0 for neither. The best line wins, so
// a general's unit can have its own line beside the base unit's.
int nameMatch(std::string_view entry, std::string_view name);

} // namespace CoH::DirectionalArmor
