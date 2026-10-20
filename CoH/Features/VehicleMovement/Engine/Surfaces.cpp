// CoH VehicleMovement: the map's surfaces, from its terrain tiles and its roads. See Internal.h.
//
// Each terrain tile is a Terrain.ini texture (AsphaltType1), with a class (ASPHALT, SNOW_FLAT, ...);
// TheTerrainVisual answers for it in -headless games too. Roads are drawn over the tiles, so they
// aren't in the tiles: the map keeps them as pairs of road points in its object list for the whole
// game, which game logic reads here. Both come from the map file and the CRC-checked INI, so every
// machine and every replay reads the same grid.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/MapObject.h"
#include "Common/TerrainTypes.h"
#include "GameClient/TerrainRoads.h"
#include "GameClient/TerrainVisual.h"
#include "GameLogic/Object.h"
#include "GameLogic/TerrainLogic.h"

#include "Kernel/Log.h"

#include "Features/VehicleMovement/Domain/Names.h"
#include "Features/VehicleMovement/Engine/Config.h"
#include "Features/VehicleMovement/Engine/Internal.h"

#include <cmath>
#include <unordered_map>

namespace CoH::VehicleMovement
{

namespace
{

SurfaceGrid &theGrid()
{
	static SurfaceGrid grid;
	return grid;
}

// The surface on a bridge, found once a map.
std::uint8_t &theBridgeSurface()
{
	static std::uint8_t surface = SurfaceGrid::NO_SURFACE;
	return surface;
}

// A tile's Tile line (by its name), else its Ground line (by its class), else the default surface.
std::uint8_t tileSurfaceOf(TerrainType *tile)	// TerrainType's getters aren't const
{
	const Config &c = config();
	if (tile == nullptr)
		return surfaceIndex(c.defaultSurface);
	for (const TileLine &line : c.tiles)
	{
		if (startsWith(line.prefix.str(), tile->getName().str()))
			return surfaceIndex(line.surface);
	}
	for (const GroundLine &line : c.grounds)
	{
		if (line.terrain == tile->getClass())
			return surfaceIndex(line.surface);
	}
	return surfaceIndex(c.defaultSurface);
}

std::uint8_t roadSurfaceOf(const AsciiString &road)
{
	for (const RoadLine &line : config().roads)
	{
		if (startsWith(line.prefix.str(), road.str()))
			return surfaceIndex(line.surface);
	}
	return SurfaceGrid::NO_SURFACE;
}

Point2 pointOf(const MapObject &mapObject)
{
	const Coord3D &location = *mapObject.getLocation();
	return { location.x, location.y };
}

int cellsOver(Real length)
{
	return static_cast<int>(std::ceil(length / MAP_XY_FACTOR));
}

void readTiles(SurfaceGrid &grid)
{
	// Looked up once per tile type, never walked.
	std::unordered_map<TerrainType *, std::uint8_t> surfaceOfTile;
	for (int row = 0; row < grid.rows(); ++row)
	{
		for (int column = 0; column < grid.columns(); ++column)
		{
			const Point2 centre = grid.cellCentre(column, row);
			TerrainType *tile = TheTerrainVisual->getTerrainTile(centre.x, centre.y);
			auto [place, isNew] = surfaceOfTile.try_emplace(tile, SurfaceGrid::NO_SURFACE);
			if (isNew)
				place->second = tileSurfaceOf(tile);
			grid.set(column, row, place->second);
		}
	}
}

// Road segments are two map objects in a row: a FLAG_ROAD_POINT1 and the FLAG_ROAD_POINT2 after it,
// named after their Roads.ini road (as W3DRoadBuffer::addMapObjects pairs them to draw them).
int readRoads(SurfaceGrid &grid)
{
	int painted = 0;
	for (MapObject *start = MapObject::getFirstMapObject(); start != nullptr; start = start->getNext())
	{
		const MapObject *end = start->getNext();
		if (!start->getFlag(FLAG_ROAD_POINT1) || end == nullptr || !end->getFlag(FLAG_ROAD_POINT2))
			continue;
		TerrainRoadType *road = TheTerrainRoads->findRoad(start->getName());
		if (road == nullptr || road->isBridge())
			continue;
		const std::uint8_t surface = roadSurfaceOf(start->getName());
		if (surface == SurfaceGrid::NO_SURFACE)
			continue;
		const float halfWidth = road->getRoadWidth() * road->getRoadWidthInTexture() / 2.0f;
		grid.paintStrip(pointOf(*start), pointOf(*end), halfWidth, surface);
		++painted;
	}
	return painted;
}

void logSurfaces(const SurfaceGrid &grid, int roads)
{
	if (!isLogging())
		return;
	const std::vector<SurfaceLine> &surfaces = config().surfaces;
	std::vector<int> counts(surfaces.size() + 1, 0);
	for (int row = 0; row < grid.rows(); ++row)
	{
		for (int column = 0; column < grid.columns(); ++column)
		{
			const std::uint8_t surface = grid.cell(column, row);
			++counts[surface < surfaces.size() ? surface : surfaces.size()];
		}
	}
	log("VehicleMovement: map surfaces, %d by %d cells, %d road segments", grid.columns(), grid.rows(), roads);
	for (size_t i = 0; i < surfaces.size(); ++i)
		log("VehicleMovement:   %s %d cells", surfaces[i].name.str(), counts[i]);
	log("VehicleMovement:   full speed %d cells", counts[surfaces.size()]);
}

} // namespace

void readSurfaces()
{
	SurfaceGrid &grid = theGrid();
	grid.clear();
	theBridgeSurface() = surfaceIndex(config().bridgeSurface);
	if (config().surfaces.empty() || TheTerrainLogic == nullptr || TheTerrainVisual == nullptr)
		return;
	Region3D extent;
	TheTerrainLogic->getExtent(&extent);
	grid.reset({ extent.lo.x, extent.lo.y }, cellsOver(extent.hi.x - extent.lo.x), cellsOver(extent.hi.y - extent.lo.y),
		MAP_XY_FACTOR);
	readTiles(grid);
	const int roads = TheTerrainRoads != nullptr ? readRoads(grid) : 0;
	logSurfaces(grid, roads);
}

void forgetSurfaces()
{
	theGrid().clear();
	theBridgeSurface() = SurfaceGrid::NO_SURFACE;
}

float surfaceSpeedOf(const Object &unit, Drive drive)
{
	const std::vector<SurfaceLine> &surfaces = config().surfaces;
	const Coord3D &position = *unit.getPosition();
	const std::uint8_t index = unit.getLayer() != LAYER_GROUND ? theBridgeSurface() : theGrid().at({ position.x, position.y });
	return index < surfaces.size() ? speedOn(surfaces[index].speed, drive) : 1.0f;
}

} // namespace CoH::VehicleMovement
