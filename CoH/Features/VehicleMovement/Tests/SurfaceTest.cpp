// CoH VehicleMovement: the surface grid, road strips and road names.

#include "Features/VehicleMovement/Domain/Names.h"
#include "Features/VehicleMovement/Domain/Surface.h"

#include <gtest/gtest.h>

using namespace CoH::VehicleMovement;

namespace
{

SurfaceGrid tenByTen()
{
	SurfaceGrid grid;
	grid.reset({ 0.0f, 0.0f }, 10, 10, 10.0f);
	return grid;
}

} // namespace

TEST(VehicleMovementSurface, WheelsAndTreadsReadTheirOwnFactor)
{
	const SurfaceSpeed asphalt = { .wheels = 1.2f, .treads = 1.05f };
	EXPECT_FLOAT_EQ(speedOn(asphalt, Drive::Wheels), 1.2f);
	EXPECT_FLOAT_EQ(speedOn(asphalt, Drive::Treads), 1.05f);
}

TEST(VehicleMovementSurface, DistanceToSegment)
{
	EXPECT_FLOAT_EQ(squaredDistanceToSegment({ 5.0f, 3.0f }, { 0.0f, 0.0f }, { 10.0f, 0.0f }), 9.0f);
	EXPECT_FLOAT_EQ(squaredDistanceToSegment({ -4.0f, 3.0f }, { 0.0f, 0.0f }, { 10.0f, 0.0f }), 25.0f);	// past the end
	EXPECT_FLOAT_EQ(squaredDistanceToSegment({ 3.0f, 4.0f }, { 0.0f, 0.0f }, { 0.0f, 0.0f }), 25.0f);	// a point
}

TEST(VehicleMovementSurface, EmptyGridHasNoSurface)
{
	const SurfaceGrid grid;
	EXPECT_EQ(grid.at({ 5.0f, 5.0f }), SurfaceGrid::NO_SURFACE);
}

TEST(VehicleMovementSurface, CellsByPosition)
{
	SurfaceGrid grid = tenByTen();
	grid.set(2, 3, 4);
	EXPECT_EQ(grid.at({ 25.0f, 35.0f }), 4);
	EXPECT_EQ(grid.at({ 29.9f, 30.0f }), 4);
	EXPECT_EQ(grid.at({ 30.0f, 35.0f }), SurfaceGrid::NO_SURFACE);
	EXPECT_EQ(grid.at({ -1.0f, 35.0f }), SurfaceGrid::NO_SURFACE);	// off the map
	EXPECT_EQ(grid.at({ 25.0f, 200.0f }), SurfaceGrid::NO_SURFACE);
}

TEST(VehicleMovementSurface, RoadStripCoversItsWidth)
{
	SurfaceGrid grid = tenByTen();
	grid.paintStrip({ 0.0f, 50.0f }, { 100.0f, 50.0f }, 12.0f, 1);	// along y = 50, 24 wide
	EXPECT_EQ(grid.at({ 50.0f, 45.0f }), 1);	// cell centre 5 off the line
	EXPECT_EQ(grid.at({ 50.0f, 55.0f }), 1);
	EXPECT_EQ(grid.at({ 50.0f, 35.0f }), SurfaceGrid::NO_SURFACE);	// 15 off
	EXPECT_EQ(grid.at({ 50.0f, 65.0f }), SurfaceGrid::NO_SURFACE);
}

TEST(VehicleMovementSurface, DiagonalRoadAndPaintingOver)
{
	SurfaceGrid grid = tenByTen();
	for (int row = 0; row < grid.rows(); ++row)
		for (int column = 0; column < grid.columns(); ++column)
			grid.set(column, row, 7);	// sand everywhere
	grid.paintStrip({ 0.0f, 0.0f }, { 100.0f, 100.0f }, 8.0f, 2);
	EXPECT_EQ(grid.at({ 55.0f, 55.0f }), 2);
	EXPECT_EQ(grid.at({ 85.0f, 15.0f }), 7);
}

TEST(VehicleMovementSurface, StripOffTheMapIsClipped)
{
	SurfaceGrid grid = tenByTen();
	grid.paintStrip({ -50.0f, 5.0f }, { 500.0f, 5.0f }, 6.0f, 3);
	EXPECT_EQ(grid.at({ 5.0f, 5.0f }), 3);
	EXPECT_EQ(grid.at({ 95.0f, 5.0f }), 3);
}

TEST(VehicleMovementNames, RoadPrefixes)
{
	EXPECT_TRUE(startsWith("TwoLane", "TwoLaneOld2Snow"));
	EXPECT_TRUE(startsWith("dirtroad", "DirtRoadTracks"));
	EXPECT_FALSE(startsWith("DirtRoad", "Dirt"));
	EXPECT_FALSE(startsWith("", "TwoLane"));
}
