// CoH VehicleMovement: what a vehicle drives on, as pure code. Unit tested in Tests/SurfaceTest.cpp.
//
// The map is cut into square cells, each holding the index of the surface it is (asphalt, sand,
// rock, ...) or NO_SURFACE. Roads are painted over the ground as strips: every cell whose centre is
// within the road's half width of its centre line takes the road's surface.

#pragma once

#include <cstdint>
#include <vector>

namespace CoH::VehicleMovement
{

// How fast each kind of drive goes on a surface, as factors on its top speed.
struct SurfaceSpeed
{
	float wheels = 1.0f;	///< four wheels and motorcycles
	float treads = 1.0f;	///< tracks, and anything else on the ground
};

enum class Drive { Wheels, Treads };

float speedOn(const SurfaceSpeed &surface, Drive drive);

struct Point2
{
	float x = 0.0f;
	float y = 0.0f;
};

// The distance from 'p' to the segment from 'a' to 'b'. Squared, so no square root is needed.
float squaredDistanceToSegment(Point2 p, Point2 a, Point2 b);

class SurfaceGrid
{
public:
	static constexpr std::uint8_t NO_SURFACE = 0xFF;

	// 'columns' by 'rows' cells of 'cellSize', the first one's corner at 'origin'; all NO_SURFACE.
	void reset(Point2 origin, int columns, int rows, float cellSize);
	void clear() { m_cells.clear(); m_columns = m_rows = 0; }
	bool empty() const { return m_cells.empty(); }

	int columns() const { return m_columns; }
	int rows() const { return m_rows; }
	Point2 cellCentre(int column, int row) const;

	void set(int column, int row, std::uint8_t surface);
	std::uint8_t cell(int column, int row) const;	///< NO_SURFACE outside the grid

	// The surface at a point in the world; NO_SURFACE outside the grid or where none was set.
	std::uint8_t at(Point2 point) const;

	// Paints a road from 'a' to 'b': each cell whose centre is within 'halfWidth' of the segment.
	void paintStrip(Point2 a, Point2 b, float halfWidth, std::uint8_t surface);

private:
	int columnOf(float x) const;
	int rowOf(float y) const;

	std::vector<std::uint8_t> m_cells;
	Point2 m_origin;
	int m_columns = 0;
	int m_rows = 0;
	float m_cellSize = 10.0f;
};

} // namespace CoH::VehicleMovement
