// CoH VehicleMovement: what the slice's Engine files share. Not for other slices; they use
// VehicleMovement.h.

#pragma once

#include "Common/GameType.h"

#include "Features/VehicleMovement/Domain/Handling.h"
#include "Features/VehicleMovement/Domain/Surface.h"

class Object;

namespace CoH::VehicleMovement
{

// A vehicle's weight with its upgrades now, and the handling that gives it.
struct Movement
{
	float weight = 0.0f;
	Handling handling;
};

// Null for anything that isn't a weighed vehicle (infantry, aircraft, buildings). Worked out again
// whenever the vehicle's or its owner's upgrades change, so an upgrade counts the moment it is done.
const Movement *movementOf(const Object &object);

void forgetObject(ObjectID id);
void forgetAll();

// The map's surfaces (Engine/Surfaces.cpp): read from its terrain tiles and roads once it is loaded,
// the same on every machine, since both come from the map file and the CRC-checked INI.
void readSurfaces();
void forgetSurfaces();

// The factor on the unit's top speed for what it is driving on now; 1 where no Surface applies.
float surfaceSpeedOf(const Object &unit, Drive drive);

} // namespace CoH::VehicleMovement
