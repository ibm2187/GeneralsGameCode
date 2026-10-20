// CoH VehicleMovement: what other slices may ask about vehicles' weight. The only header of this slice
// others include.
//
// Every ground vehicle has a weight in tons, its own and what its upgrades add (Composite Armor, the
// Overlord's riders). Weight scales its top speed, acceleration, braking, turn rate and how much a
// hill slows it. While the VehicleMovement flag is off every vehicle drives as in retail.

#pragma once

#include <optional>

class Object;

namespace CoH::VehicleMovement
{

bool isActive();

// The vehicle's weight with its upgrades now, in tons; nothing for what isn't a weighed vehicle
// (infantry, aircraft, buildings) or while the flag is off.
std::optional<float> weightOf(const Object &vehicle);

} // namespace CoH::VehicleMovement
