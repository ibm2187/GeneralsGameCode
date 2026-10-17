// CoH Sectors: the borders' drawing geometry, worked out ahead so drawing only picks colors.
// - Sector shapes never change during a game, so each border between two sectors, and each sector's
//   own inset edge, is made once, the first time it is drawn after the World is built (a new game or
//   a loaded one).
// - The outline round each owner's territory is made again only when a sector changes hands.

#pragma once

#include "Lib/BaseType.h"

#include "Features/Sectors/Domain/BorderLines.h"
#include "Features/Sectors/Domain/Types.h"

#include <vector>

namespace CoH::Sectors
{

class World;

// A strip of ground line, in world units, lifted onto the terrain. Long strips are cut into short
// chunks with a bounding box, so off-screen ones are skipped.
struct GroundStrip
{
	enum class Kind
	{
		Center,	///< on a border between sectors 'side' and 'other'; drawn when they have one owner
		Flash,	///< just inside sector 'side', along its border with 'other'; drawn when it changes hands
		Edge,	///< the crisp line just inside the territory of 'owner'
		Band,	///< the soft band inside the territory of 'owner': from each point to 'inner', fading
	};

	Kind kind = Kind::Center;
	int side = -1;
	int other = -1;
	PlayerId owner = NoPlayer;
	int firstIndex = 0;		///< where the chunk starts along its whole strip, so dashes line up
	std::vector<Coord3D> points;
	std::vector<Coord3D> inner;	///< a band's inner edge, one point for each of 'points'
	Real minX = 0, minY = 0, maxX = 0, maxY = 0;
};

// Made once per World build.
struct SectorGeometry
{
	int buildNumber = -1;
	std::vector<BorderLine> lines;		///< smooth, flat; the minimap draws these
	std::vector<GroundStrip> strips;	///< Center, then Flash
};

// Made again when the owners change.
struct TerritoryGeometry
{
	int buildNumber = -1;
	std::vector<PlayerId> owners;
	std::vector<GroundStrip> strips;	///< the bands, then the edges
};

// A point lifted onto the ground, or onto the water where there is water, a little above it.
Coord3D onGround(Vec2 point);

const SectorGeometry &sectorGeometry(const World &w);
const TerritoryGeometry &territoryGeometry(const World &w);

} // namespace CoH::Sectors
