// CoH VehicleMovement: the tuning, read from the CoHVehicleMovement block of the pack
// (Data/INI/10_VehicleMovement.ini). Back to these defaults after every game.

#pragma once

#include "Common/AsciiString.h"
#include "Common/KindOf.h"
#include "Common/TerrainTypes.h"

#include "Features/VehicleMovement/Domain/Handling.h"
#include "Features/VehicleMovement/Domain/Surface.h"

#include <vector>

class INI;

namespace CoH::VehicleMovement
{

// A Class line: a weight class and its tons.
struct WeightClass
{
	AsciiString name;
	float tons = 0.0f;
};

// A Unit line: a vehicle's class, or its own tons when 'tons' is 0 or more.
struct UnitLine
{
	AsciiString name;
	AsciiString weightClass;
	float tons = -1.0f;
};

// An Upgrade line: the tons an upgrade adds.
struct UpgradeLine
{
	AsciiString name;
	float tons = 0.0f;
};

// A Surface line: a kind of ground and how fast each drive goes on it.
struct SurfaceLine
{
	AsciiString name;
	SurfaceSpeed speed;
};

// A Ground line: the surface of a terrain texture class (Terrain.ini's Class).
struct GroundLine
{
	TerrainClass terrain = TERRAIN_NONE;
	AsciiString surface;
};

// A Tile line: the surface of terrain textures whose Terrain.ini name starts with 'prefix'. Tile lines
// win over Ground lines.
struct TileLine
{
	AsciiString prefix;
	AsciiString surface;
};

// A Road line: the surface of the map's roads whose Roads.ini name starts with 'prefix'.
struct RoadLine
{
	AsciiString prefix;
	AsciiString surface;
};

struct Config
{
	std::vector<WeightClass> classes;
	AsciiString defaultClass = "Medium";	///< for vehicles no Unit line names
	std::vector<UnitLine> units;
	std::vector<UpgradeLine> upgrades;
	HandlingCurves curves;
	float slopeLookAhead = 20.0f;			///< how far ahead the ground's rise is measured
	KindOfMaskType vehicles;				///< objects of these kinds have a weight ...
	KindOfMaskType exempt;					///< ... unless they are also one of these

	std::vector<SurfaceLine> surfaces;
	std::vector<TileLine> tiles;
	std::vector<GroundLine> grounds;
	std::vector<RoadLine> roads;
	AsciiString defaultSurface;				///< ground no Ground line names; none: full speed
	AsciiString bridgeSurface;				///< on a bridge; none: full speed

	Config();
};

const Config &config();
void parseConfigBlock(INI *ini);
void resetConfig();

// The index of the Surface line named, or SurfaceGrid::NO_SURFACE.
std::uint8_t surfaceIndex(const AsciiString &name);

// The tons of the class named, or of the default class for a name no Class line has.
float classTons(const AsciiString &name);

} // namespace CoH::VehicleMovement
