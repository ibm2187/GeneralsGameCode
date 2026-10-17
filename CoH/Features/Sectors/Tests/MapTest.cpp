// CoH Sectors: tests for turning a map's supply sources into points and dividing the ground.

#include <gtest/gtest.h>

#include "Features/Sectors/Domain/AiTargets.h"
#include "Features/Sectors/Domain/Points.h"
#include "Features/Sectors/Domain/SectorMap.h"

using namespace CoH::Sectors;

TEST(SectorsPoints, EveryStartGetsAHomePointFirst)
{
	const std::vector<PointPlan> points = planPoints({}, { { { 100, 100 }, 3 }, { { 900, 900 }, 5 } }, PointRules());
	ASSERT_EQ(points.size(), 2u);
	EXPECT_EQ(points[0].kind, SectorKind::Home);
	EXPECT_EQ(points[0].homePlayer, 3);
	EXPECT_EQ(points[1].homePlayer, 5);
}

TEST(SectorsPoints, NearbySourcesFormOnePointInTheirMiddle)
{
	PointRules rules;
	rules.mergeDistance = 100.0f;
	const std::vector<Source> sources = {
		{ { 1000, 1000 }, SectorKind::Supply, 11 },
		{ { 3000, 3000 }, SectorKind::Supply, 12 },
		{ { 1080, 1000 }, SectorKind::Supply, 13 },
		{ { 1160, 1000 }, SectorKind::HighValue, 14 },	// joins through 13: a chain
	};
	const std::vector<PointPlan> points = planPoints(sources, {}, rules);
	ASSERT_EQ(points.size(), 2u);
	EXPECT_EQ(points[0].members, (std::vector<int>{ 11, 13, 14 }));
	EXPECT_FLOAT_EQ(points[0].position.x, 1080.0f);
	EXPECT_EQ(points[0].kind, SectorKind::HighValue);
	EXPECT_EQ(points[1].members, (std::vector<int>{ 12 }));
	EXPECT_EQ(points[1].kind, SectorKind::Supply);
}

TEST(SectorsPoints, SourcesNearAStartJoinItsHome)
{
	PointRules rules;
	rules.mergeDistance = 50.0f;
	rules.homeMergeDistance = 300.0f;
	const std::vector<Source> sources = { { { 300, 100 }, SectorKind::HighValue, 7 }, { { 2000, 100 }, SectorKind::Supply, 8 } };
	const std::vector<PointPlan> points = planPoints(sources, { { { 100, 100 }, 0 } }, rules);
	ASSERT_EQ(points.size(), 2u);
	EXPECT_EQ(points[0].kind, SectorKind::Home);
	EXPECT_EQ(points[0].members, (std::vector<int>{ 7 }));
	EXPECT_FLOAT_EQ(points[0].position.x, 100.0f);	// the home stays at the start
	EXPECT_EQ(points[1].members, (std::vector<int>{ 8 }));
}

TEST(SectorsMap, TheNearestPointOwnsTheGround)
{
	MapRules rules;
	rules.cellSize = 10.0f;
	rules.homeRadius = 0.0f;
	const SectorMap map = SectorMap::build({ { { 100, 50 } }, { { 300, 50 } } }, 400.0f, 100.0f, rules);
	ASSERT_EQ(map.sectorCount(), 2);
	EXPECT_EQ(map.sectorAt({ 50, 50 }), 0);
	EXPECT_EQ(map.sectorAt({ 190, 50 }), 0);
	EXPECT_EQ(map.sectorAt({ 210, 50 }), 1);
	EXPECT_EQ(map.sectorAt({ 5000, -40 }), 1);	// off the map: the nearest edge cell
	EXPECT_EQ(map.cellCount(0) + map.cellCount(1), 40 * 10);
}

TEST(SectorsMap, HomesKeepTheirRadius)
{
	MapRules rules;
	rules.cellSize = 10.0f;
	rules.homeRadius = 150.0f;
	const SectorMap map = SectorMap::build({ { { 100, 50 }, true }, { { 200, 50 } } }, 400.0f, 100.0f, rules);
	EXPECT_EQ(map.sectorAt({ 230, 50 }), 0);	// nearer the supply point, but within the home's radius
	EXPECT_EQ(map.sectorAt({ 270, 50 }), 1);
}

TEST(SectorsMap, TouchingSectorsAreNeighbors)
{
	MapRules rules;
	rules.cellSize = 10.0f;
	rules.homeRadius = 0.0f;
	const SectorMap map = SectorMap::build({ { { 50, 50 } }, { { 150, 50 } }, { { 250, 50 } } }, 300.0f, 100.0f, rules);
	EXPECT_TRUE(map.areNeighbors(0, 1));
	EXPECT_TRUE(map.areNeighbors(1, 2));
	EXPECT_FALSE(map.areNeighbors(0, 2));
	EXPECT_EQ(map.neighbors()[1], (std::vector<int>{ 0, 2 }));
}

TEST(SectorsMap, BordersRunBetweenSectors)
{
	MapRules rules;
	rules.cellSize = 10.0f;
	rules.homeRadius = 0.0f;
	const SectorMap map = SectorMap::build({ { { 50, 50 } }, { { 150, 50 } } }, 200.0f, 100.0f, rules);
	ASSERT_EQ(map.borders().size(), 1u);	// one straight vertical line, joined from ten cell edges
	const BorderEdge &edge = map.borders()[0];
	EXPECT_FLOAT_EQ(edge.a.x, 100.0f);
	EXPECT_FLOAT_EQ(edge.a.y, 0.0f);
	EXPECT_FLOAT_EQ(edge.b.y, 100.0f);
	EXPECT_EQ(edge.left, 0);	// heading +y, the west side is on the left
	EXPECT_EQ(edge.right, 1);
}

TEST(SectorsMap, NoPointsNoMap)
{
	EXPECT_TRUE(SectorMap::build({}, 100.0f, 100.0f, MapRules()).empty());
	EXPECT_EQ(SectorMap().sectorAt({ 1, 1 }), -1);
}

TEST(SectorsAi, DefendThenReconnectThenExpandNearestFirst)
{
	// 0 home - 1 - 2 - 3 (cut off) ... and 4 far away, next to home.
	const std::vector<std::vector<int>> neighbors = { { 1, 4 }, { 0, 2 }, { 1, 3 }, { 2 }, { 0 } };
	std::vector<AiSector> sectors(5);
	sectors[0] = { { 0, 0 }, SectorKind::Home, 0 };
	sectors[1] = { { 1000, 0 }, SectorKind::Supply, 0, false, false, true };	// ours, under attack
	sectors[2] = { { 2000, 0 }, SectorKind::Supply, NoPlayer };				// joins 3 back up
	sectors[3] = { { 3000, 0 }, SectorKind::Supply, 0 };
	sectors[4] = { { 0, 1500 }, SectorKind::Supply, NoPlayer };
	const std::vector<bool> connected = { true, true, false, false, false };

	const std::vector<int> order = rankTargets(0, sectors, neighbors, connected, { 0, 0 }, AiWeights());
	EXPECT_EQ(order, (std::vector<int>{ 1, 2, 4 }));
}

TEST(SectorsAi, AlliedAndLockedSectorsAreNoTargets)
{
	const std::vector<std::vector<int>> neighbors = { { 1, 2 }, { 0 }, { 0 } };
	std::vector<AiSector> sectors(3);
	sectors[0] = { { 0, 0 }, SectorKind::Home, 0 };
	sectors[1] = { { 500, 0 }, SectorKind::Supply, 1, true };
	sectors[2] = { { 0, 500 }, SectorKind::Home, 2, false, true };
	EXPECT_TRUE(rankTargets(0, sectors, neighbors, { true, false, false }, { 0, 0 }, AiWeights()).empty());
}
