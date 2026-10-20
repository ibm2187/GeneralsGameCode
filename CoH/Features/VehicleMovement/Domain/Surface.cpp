// CoH VehicleMovement: see Surface.h.

#include "Features/VehicleMovement/Domain/Surface.h"

#include <algorithm>
#include <cstddef>

namespace CoH::VehicleMovement
{

float speedOn(const SurfaceSpeed &surface, Drive drive)
{
	return drive == Drive::Wheels ? surface.wheels : surface.treads;
}

float squaredDistanceToSegment(Point2 p, Point2 a, Point2 b)
{
	const float dx = b.x - a.x;
	const float dy = b.y - a.y;
	const float length2 = dx * dx + dy * dy;
	float share = 0.0f;
	if (length2 > 0.0f)
		share = std::clamp(((p.x - a.x) * dx + (p.y - a.y) * dy) / length2, 0.0f, 1.0f);
	const float nearX = a.x + dx * share - p.x;
	const float nearY = a.y + dy * share - p.y;
	return nearX * nearX + nearY * nearY;
}

void SurfaceGrid::reset(Point2 origin, int columns, int rows, float cellSize)
{
	m_origin = origin;
	m_columns = std::max(columns, 0);
	m_rows = std::max(rows, 0);
	m_cellSize = cellSize > 0.0f ? cellSize : 10.0f;
	m_cells.assign(static_cast<size_t>(m_columns) * static_cast<size_t>(m_rows), NO_SURFACE);
}

Point2 SurfaceGrid::cellCentre(int column, int row) const
{
	return { m_origin.x + (column + 0.5f) * m_cellSize, m_origin.y + (row + 0.5f) * m_cellSize };
}

void SurfaceGrid::set(int column, int row, std::uint8_t surface)
{
	if (column >= 0 && column < m_columns && row >= 0 && row < m_rows)
		m_cells[static_cast<size_t>(row) * m_columns + column] = surface;
}

std::uint8_t SurfaceGrid::cell(int column, int row) const
{
	if (column < 0 || column >= m_columns || row < 0 || row >= m_rows)
		return NO_SURFACE;
	return m_cells[static_cast<size_t>(row) * m_columns + column];
}

int SurfaceGrid::columnOf(float x) const
{
	const float cells = (x - m_origin.x) / m_cellSize;
	return cells < 0.0f ? -1 : static_cast<int>(cells);
}

int SurfaceGrid::rowOf(float y) const
{
	const float cells = (y - m_origin.y) / m_cellSize;
	return cells < 0.0f ? -1 : static_cast<int>(cells);
}

std::uint8_t SurfaceGrid::at(Point2 point) const
{
	return cell(columnOf(point.x), rowOf(point.y));
}

void SurfaceGrid::paintStrip(Point2 a, Point2 b, float halfWidth, std::uint8_t surface)
{
	if (halfWidth <= 0.0f || m_cells.empty())
		return;
	const int firstColumn = std::max(columnOf(std::min(a.x, b.x) - halfWidth), 0);
	const int lastColumn = std::min(columnOf(std::max(a.x, b.x) + halfWidth), m_columns - 1);
	const int firstRow = std::max(rowOf(std::min(a.y, b.y) - halfWidth), 0);
	const int lastRow = std::min(rowOf(std::max(a.y, b.y) + halfWidth), m_rows - 1);
	const float reach2 = halfWidth * halfWidth;
	for (int row = firstRow; row <= lastRow; ++row)
	{
		for (int column = firstColumn; column <= lastColumn; ++column)
		{
			if (squaredDistanceToSegment(cellCentre(column, row), a, b) <= reach2)
				set(column, row, surface);
		}
	}
}

} // namespace CoH::VehicleMovement
