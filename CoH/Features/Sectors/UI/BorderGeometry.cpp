// CoH Sectors: see BorderGeometry.h.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/TerrainLogic.h"

#include "Kernel/Log.h"

#include "Features/Sectors/Engine/Config.h"
#include "Features/Sectors/Engine/World.h"
#include "Features/Sectors/UI/BorderGeometry.h"

#include <algorithm>
#include <chrono>
#include <cmath>

namespace CoH::Sectors
{

namespace
{

const Real EDGE_INSET = 4.0f;		///< world units the crisp line sits inside its territory
const Real LIFT = 1.5f;				///< above the ground, so lines don't sink into it
const size_t CHUNK_POINTS = 24;		///< points per culled chunk

// Cuts a piece into chunks that share their end points, so the strip stays joined. A band's inner
// edge, when it has one, is cut alongside.
void addStrip(std::vector<GroundStrip> &strips, const GroundStrip &shape, const LinePiece &piece,
	const std::vector<Vec2> &inner = {})
{
	std::vector<Vec2> points = piece.points;
	std::vector<Vec2> inside = inner;
	if (piece.closed && !points.empty())
	{
		points.push_back(points.front());
		if (!inside.empty())
			inside.push_back(inside.front());
	}

	for (size_t start = 0; start + 1 < points.size(); start += CHUNK_POINTS - 1)
	{
		const size_t end = std::min(points.size(), start + CHUNK_POINTS);
		GroundStrip chunk = shape;
		chunk.firstIndex = (int)start;
		chunk.minX = chunk.minY = 1e9f;
		chunk.maxX = chunk.maxY = -1e9f;
		for (size_t i = start; i < end; ++i)
		{
			chunk.points.push_back(onGround(points[i]));
			chunk.minX = std::min(chunk.minX, points[i].x);
			chunk.minY = std::min(chunk.minY, points[i].y);
			chunk.maxX = std::max(chunk.maxX, points[i].x);
			chunk.maxY = std::max(chunk.maxY, points[i].y);
			if (!inside.empty())
				chunk.inner.push_back(onGround(inside[i]));
		}
		strips.push_back(std::move(chunk));
	}
}

void build(SectorGeometry &geometry, const World &w)
{
	const auto started = std::chrono::steady_clock::now();
	geometry = SectorGeometry();
	geometry.buildNumber = w.buildNumber();
	geometry.lines = smoothBorders(w.map(), config().borderLines);

	std::vector<GroundStrip> flashes;
	for (const BorderLine &line : geometry.lines)
	{
		GroundStrip shape;
		shape.kind = GroundStrip::Kind::Center;
		shape.side = line.left;
		shape.other = line.right;
		addStrip(geometry.strips, shape, { line.points, line.closed });

		shape.kind = GroundStrip::Kind::Flash;
		for (int side : { line.left, line.right })
		{
			shape.side = side;
			shape.other = side == line.left ? line.right : line.left;
			for (const LinePiece &piece : insetLine(line, side, EDGE_INSET, w.map()))
				addStrip(flashes, shape, piece);
		}
	}
	geometry.strips.insert(geometry.strips.end(), flashes.begin(), flashes.end());

	if (CoH::isLogging())
	{
		const auto took = std::chrono::steady_clock::now() - started;
		CoH::log("Sectors: border lines made: %d lines, %d chunks, %.1f ms", (int)geometry.lines.size(),
			(int)geometry.strips.size(), std::chrono::duration<double, std::milli>(took).count());
	}
}

void build(TerritoryGeometry &geometry, const World &w)
{
	const auto started = std::chrono::steady_clock::now();
	geometry = TerritoryGeometry();
	geometry.buildNumber = w.buildNumber();
	geometry.owners = w.ownersAtRefresh();
	const Real bandWidth = config().borderBandWidth - EDGE_INSET;

	// Every neutral sector is in the region NoPlayer, so the neutral ground has one outline too.
	const std::vector<RegionOutline> outlines = outlineRegions(w.map(), geometry.owners, config().borderLines);

	std::vector<GroundStrip> edges;
	for (const RegionOutline &outline : outlines)
	{
		GroundStrip shape;
		shape.owner = outline.region;
		for (const LinePiece &piece : insetOutline(outline, EDGE_INSET, w.map()))
		{
			shape.kind = GroundStrip::Kind::Edge;
			addStrip(edges, shape, piece);
			if (outline.region == NoPlayer || bandWidth <= 0.0f)
				continue;
			// The band runs from the crisp line to its inner edge, further in.
			shape.kind = GroundStrip::Kind::Band;
			addStrip(geometry.strips, shape, piece, offsetLine(piece.points, piece.closed, bandDepths(piece.points, piece.closed, bandWidth)));
		}
	}
	geometry.strips.insert(geometry.strips.end(), edges.begin(), edges.end());

	if (CoH::isLogging())
	{
		const auto took = std::chrono::steady_clock::now() - started;
		CoH::log("Sectors: territory lines made again: %d outlines, %d chunks, %.1f ms", (int)outlines.size(),
			(int)geometry.strips.size(), std::chrono::duration<double, std::milli>(took).count());
	}
}

} // namespace

Coord3D onGround(Vec2 point)
{
	Real water = 0.0f;
	Coord3D world = { point.x, point.y, TheTerrainLogic->getGroundHeight(point.x, point.y) };
	if (TheTerrainLogic->isUnderwater(point.x, point.y, &water))
		world.z = std::max(world.z, water);
	world.z += LIFT;
	return world;
}

const SectorGeometry &sectorGeometry(const World &w)
{
	static SectorGeometry geometry;
	if (geometry.buildNumber != w.buildNumber())
		build(geometry, w);
	return geometry;
}

const TerritoryGeometry &territoryGeometry(const World &w)
{
	static TerritoryGeometry geometry;
	if (geometry.buildNumber != w.buildNumber() || geometry.owners != w.ownersAtRefresh())
		build(geometry, w);
	return geometry;
}

} // namespace CoH::Sectors
