// CoH Sectors: the map divided into sectors. The ground is a grid of square cells; each cell belongs
// to the point nearest to it, so the whole map is divided. Ground close to a home point always belongs
// to that home, so a base has room to grow. Sectors whose cells touch are neighbors.

#pragma once

#include "Types.h"

#include <vector>

namespace CoH::Sectors
{

struct Site
{
	Vec2 position;
	bool home = false;
};

struct MapRules
{
	float cellSize = 40.0f;		///< world units per side of a grid cell
	float homeRadius = 350.0f;	///< ground this close to a home point is always that home's
};

// A straight piece of border between two sectors. Seen from a to b, 'left' is on the left.
struct BorderEdge
{
	Vec2 a;
	Vec2 b;
	int left = -1;
	int right = -1;
};

class SectorMap
{
public:
	SectorMap() = default;

	// The map runs from (0, 0) to (width, height) in world units. Sector i is the ground of site i.
	static SectorMap build(const std::vector<Site> &sites, float width, float height, const MapRules &rules);

	int sectorCount() const { return (int)m_neighbors.size(); }
	bool empty() const { return m_cells.empty(); }

	// The sector at a world position; positions off the map count as the nearest edge cell.
	int sectorAt(Vec2 position) const;

	// Each sector's neighbors, in increasing order.
	const std::vector<std::vector<int>> &neighbors() const { return m_neighbors; }
	bool areNeighbors(int a, int b) const;

	// Every border between two sectors, as long straight edges along the grid.
	const std::vector<BorderEdge> &borders() const { return m_borders; }

	// How many cells each sector has (its area in cells).
	int cellCount(int sector) const;

	int gridWidth() const { return m_width; }
	int gridHeight() const { return m_height; }
	float cellSize() const { return m_cellSize; }
	int cellSector(int x, int y) const { return m_cells[y * m_width + x]; }

private:
	void assignCells(const std::vector<Site> &sites, float homeRadius);
	void findNeighbors();
	void findBorders();

	int m_width = 0;
	int m_height = 0;
	float m_cellSize = 1.0f;
	std::vector<int> m_cells;	///< sector of each cell, row by row from (0, 0)
	std::vector<std::vector<int>> m_neighbors;
	std::vector<BorderEdge> m_borders;
};

} // namespace CoH::Sectors
