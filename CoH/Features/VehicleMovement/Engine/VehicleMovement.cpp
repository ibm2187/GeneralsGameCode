// CoH VehicleMovement: the slice's declaration, and weight applied to the handling the engine asks
// for: top speed, the slope and the surface through hub unitSpeed, acceleration, braking and turn
// rate through hub unitHandling. See CoH/Features/VehicleMovement/README.md.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/AIPathfind.h"
#include "GameLogic/Locomotor.h"
#include "GameLogic/Module/AIUpdate.h"
#include "GameLogic/Object.h"
#include "GameLogic/TerrainLogic.h"

#include "Kernel/Kernel.h"

#include "Features/VehicleMovement/Engine/Config.h"
#include "Features/VehicleMovement/Engine/Internal.h"
#include "Features/VehicleMovement/VehicleMovement.h"

#include <optional>

namespace CoH::VehicleMovement
{

namespace
{

const Feature *&theFeature()
{
	static const Feature *feature = nullptr;
	return feature;
}

// How a ground vehicle drives: on wheels (four, or a motorcycle's two) or on treads (and anything else
// on the ground). Hover, wings, thrust and legs drive as they did.
std::optional<Drive> driveOf(const Object &unit)
{
	const AIUpdateInterface *ai = unit.getAI();
	const Locomotor *locomotor = ai != nullptr ? ai->getCurLocomotor() : nullptr;
	if (locomotor == nullptr)
		return std::nullopt;
	switch (locomotor->getAppearance())
	{
		case LOCO_WHEELS_FOUR:
		case LOCO_MOTORCYCLE:
			return Drive::Wheels;
		case LOCO_TREADS:
		case LOCO_OTHER:
			return Drive::Treads;
		default:
			return std::nullopt;
	}
}

const Movement *drivenMovementOf(const Object &unit)
{
	return driveOf(unit) ? movementOf(unit) : nullptr;
}

// The ground's rise ahead of the vehicle, over the distance to the point ahead: 0.3 is a 30% grade.
float gradeAhead(const Object &unit)
{
	if (unit.getLayer() != LAYER_GROUND || unit.isSignificantlyAboveTerrain())
		return 0.0f;
	const float distance = config().slopeLookAhead;
	if (distance <= 0.0f)
		return 0.0f;
	const Coord3D &here = *unit.getPosition();
	const Coord3D &facing = *unit.getUnitDirectionVector2D();
	const float heightHere = TheTerrainLogic->getGroundHeight(here.x, here.y);
	const float heightAhead = TheTerrainLogic->getGroundHeight(here.x + facing.x * distance, here.y + facing.y * distance);
	return (heightAhead - heightHere) / distance;
}

void scaleSpeed(const Object *unit, float &speed)
{
	const std::optional<Drive> drive = driveOf(*unit);
	const Movement *movement = drive ? movementOf(*unit) : nullptr;
	if (movement == nullptr)
		return;
	const Handling &handling = movement->handling;
	speed *= handling.speed * surfaceSpeedOf(*unit, *drive);
	if (handling.slopePenalty > 0.0f)
		speed *= slopeSpeedFactor(gradeAhead(*unit), handling.slopePenalty, config().curves.slopeMinSpeed);
}

void scaleHandling(const Object *unit, Hooks::Handling which, float &value)
{
	const Movement *movement = drivenMovementOf(*unit);
	if (movement == nullptr)
		return;
	switch (which)
	{
		case Hooks::Handling::Acceleration: value *= movement->handling.acceleration; break;
		case Hooks::Handling::Braking: value *= movement->handling.braking; break;
		case Hooks::Handling::TurnRate: value *= movement->handling.turnRate; break;
	}
}

void onObjectDestroyed(Object *object)
{
	forgetObject(object->getID());
}

void onGameStart(bool)
{
	forgetAll();
	readSurfaces();
}

void init(Feature &self)
{
	theFeature() = &self;
	registerIniBlock("CoHVehicleMovement", parseConfigBlock);
	Hooks::unitSpeed().subscribe(self, scaleSpeed);
	Hooks::unitHandling().subscribe(self, scaleHandling);
	Hooks::objectDestroyed().subscribe(self, onObjectDestroyed);
	Hooks::gameStartAfterMap().subscribe(self, onGameStart);
}

void reset()
{
	forgetAll();
	forgetSurfaces();
	resetConfig();
}

} // namespace

bool isActive()
{
	return theFeature() != nullptr && theFeature()->isActive();
}

std::optional<float> weightOf(const Object &vehicle)
{
	if (!isActive())
		return std::nullopt;
	const Movement *movement = movementOf(vehicle);
	return movement != nullptr ? std::optional<float>(movement->weight) : std::nullopt;
}

} // namespace CoH::VehicleMovement

COH_FEATURE(VehicleMovement, CoH::VehicleMovement::init, CoH::VehicleMovement::reset)
