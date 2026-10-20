// CoH VehicleMovement: which Unit or Road line names an object or a road, as pure code. Unit tested in
// Tests/HandlingTest.cpp and Tests/SurfaceTest.cpp.

#pragma once

#include <string_view>

namespace CoH::VehicleMovement
{

// How well a line naming 'entry' covers the object 'name': 2 for the name itself, 1 for a general's
// version of it ("ChinaTankOverlord" covers "Tank_ChinaTankOverlord"), 0 for neither. Case doesn't
// matter. The best line wins, so a general's unit can have its own line beside the base unit's.
int nameMatch(std::string_view entry, std::string_view name);

// Does 'name' start with 'entry', without case? A Road line names the start of road names:
// "TwoLane" covers "TwoLaneDark" and "TwoLaneOld2Snow".
bool startsWith(std::string_view entry, std::string_view name);

} // namespace CoH::VehicleMovement
