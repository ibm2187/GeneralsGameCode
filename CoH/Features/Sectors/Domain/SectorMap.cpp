// CoH Sectors: see SectorMap.h.

#include "SectorMap.h"

#include <algorithm>
#include <cmath>

namespace CoH::Sectors
{

namespace
{

// The nearest site, preferring a home within its radius. Ties go to the lower index.
int nearestSite(const std::vector<Site> &sites, Vec2 position, float homeRadiusSquared)
{
	int nearest = -1;
	float nearestDistance = 0.0f;
	int nearestHome = -1;
	float nearestHomeDistance = homeRadiusSquared;

	for (size_t i = 0; i < sites.size(); ++i)
	{
		const float d = distanceSquared(sites[i].position, position);
		if (nearest < 0 || d < nearestDistance)
		{
			nearest = (int)i;
			nearestDistance = d;
		}
		if (sites[i].home && d <= nearestHomeDistance)
		{
			nearestHome = (int)i;
			nearestHomeDistance = d;
		}
	}
	return nearestHome >= 0 ? nearestHome : nearest;
}

int cellsAlong(float length, float cellSize)
{
	return std::max(1, (int)std::ceil(length / cellSize));
}

} // namespace

SectorMap SectorMap::build(const std::vector<Site> &sites, float width, float height, const MapRules &rules)
{
	SectorMap map;
	if (sites.empty() || width <= 0.0f || height <= 0.0f || rules.cellSize <= 0.0f)
		return map;

	map.m_cellSize = rules.cellSize;
	map.m_width = cellsAlong(width, rules.cellSize);
	map.m_height = cellsAlong(height, rules.cellSize);
	map.m_neighbors.resize(sites.size());
	map.assignCells(sites, rules.homeRadius);
	map.findNeighbors();
	map.findBorders();
	return map;
}

void SectorMap::assignCells(const std::vector<Site> &sites, float homeRadius)
{
	m_cells.resize((size_t)m_width * m_height);
	for (int y = 0; y < m_height; ++y)
	{
		for (int x = 0; x < m_width; ++x)
		{
			const Vec2 center = { (x + 0.5f) * m_cellSize, (y + 0.5f) * m_cellSize };
			m_cells[y * m_width + x] = nearestSite(sites, center, homeRadius * homeRadius);
		}
	}
}

void SectorMap::findNeighbors()
{
	const auto link = [this](int a, int b)
	{
		if (a == b)
			return;
		std::vector<int> &list = m_neighbors[a];
		if (std::find(list.begin(), list.end(), b) == list.end())
			list.push_back(b);
	};

	for (int y = 0; y < m_height; ++y)
	{
		for (int x = 0; x < m_width; ++x)
		{
			const int here = cellSector(x, y);
			if (x + 1 < m_width)
			{
				link(here, cellSector(x + 1, y));
				link(cellSector(x + 1, y), here);
			}
			if (y + 1 < m_height)
			{
				link(here, cellSector(x, y + 1));
				link(cellSector(x, y + 1), here);
			}
		}
	}

	for (std::vector<int> &list : m_neighbors)
		std::sort(list.begin(), list.end());
}

// Cell edges between two sectors, joined into runs while the pair of sectors stays the same.
void SectorMap::findBorders()
{
	// Horizontal lines between row y-1 (right of an edge heading +x) and row y (left of it).
	for (int y = 1; y < m_height; ++y)
	{
		for (int x = 0; x < m_width;)
		{
			const int below = cellSector(x, y - 1);
			const int above = cellSector(x, y);
			int end = x + 1;
			if (below != above)
			{
				while (end < m_width && cellSector(end, y - 1) == below && cellSector(end, y) == above)
					++end;
				m_borders.push_back({ { x * m_cellSize, y * m_cellSize }, { end * m_cellSize, y * m_cellSize }, above, below });
			}
			x = end;
		}
	}

	// Vertical lines between column x-1 (left of an edge heading +y) and column x (right of it).
	for (int x = 1; x < m_width; ++x)
	{
		for (int y = 0; y < m_height;)
		{
			const int west = cellSector(x - 1, y);
			const int east = cellSector(x, y);
			int end = y + 1;
			if (west != east)
			{
				while (end < m_height && cellSector(x - 1, end) == west && cellSector(x, end) == east)
					++end;
				m_borders.push_back({ { x * m_cellSize, y * m_cellSize }, { x * m_cellSize, end * m_cellSize }, west, east });
			}
			y = end;
		}
	}
}

int SectorMap::sectorAt(Vec2 position) const
{
	if (m_cells.empty())
		return -1;

	const int x = std::clamp((int)std::floor(position.x / m_cellSize), 0, m_width - 1);
	const int y = std::clamp((int)std::floor(position.y / m_cellSize), 0, m_height - 1);
	return cellSector(x, y);
}

bool SectorMap::areNeighbors(int a, int b) const
{
	if (a < 0 || a >= sectorCount())
		return false;
	const std::vector<int> &list = m_neighbors[a];
	return std::binary_search(list.begin(), list.end(), b);
}

int SectorMap::cellCount(int sector) const
{
	return (int)std::count(m_cells.begin(), m_cells.end(), sector);
}

} // namespace CoH::Sectors
