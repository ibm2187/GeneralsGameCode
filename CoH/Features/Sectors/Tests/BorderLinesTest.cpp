// CoH Sectors: tests for the smooth border lines the UI draws.

#include <gtest/gtest.h>

#include "Features/Sectors/Domain/BorderLines.h"

#include <cmath>

using namespace CoH::Sectors;

namespace
{

MapRules rulesWithCell(float cellSize)
{
	MapRules rules;
	rules.cellSize = cellSize;
	rules.homeRadius = 0.0f;
	return rules;
}

bool isNear(Vec2 a, Vec2 b, float tolerance = 0.01f)
{
	return distanceSquared(a, b) <= tolerance * tolerance;
}

// How far a point is from the line through a and b.
float distanceToLine(Vec2 point, Vec2 a, Vec2 b)
{
	const float dx = b.x - a.x;
	const float dy = b.y - a.y;
	return std::fabs((point.x - a.x) * dy - (point.y - a.y) * dx) / std::sqrt(dx * dx + dy * dy);
}

} // namespace

TEST(SectorsBorderLines, EdgesMeetingEndToEndBecomeOneLine)
{
	// A staircase between sectors 0 and 1, given out of order and with one edge facing the other way.
	const std::vector<BorderEdge> edges = {
		{ { 10, 0 }, { 10, 10 }, 0, 1 },
		{ { 0, 0 }, { 10, 0 }, 0, 1 },
		{ { 20, 10 }, { 10, 10 }, 1, 0 },
	};
	const std::vector<BorderLine> lines = chainBorders(edges);
	ASSERT_EQ(lines.size(), 1u);
	EXPECT_EQ(lines[0].left, 0);
	EXPECT_EQ(lines[0].right, 1);
	EXPECT_FALSE(lines[0].closed);
	ASSERT_EQ(lines[0].points.size(), 4u);
	EXPECT_TRUE(isNear(lines[0].points[0], { 0, 0 }));
	EXPECT_TRUE(isNear(lines[0].points[1], { 10, 0 }));
	EXPECT_TRUE(isNear(lines[0].points[2], { 10, 10 }));
	EXPECT_TRUE(isNear(lines[0].points[3], { 20, 10 }));
}

TEST(SectorsBorderLines, EachPairOfSectorsGetsItsOwnLines)
{
	const std::vector<BorderEdge> edges = {
		{ { 0, 0 }, { 10, 0 }, 0, 1 },
		{ { 10, 0 }, { 20, 0 }, 0, 2 },	// meets the first, but between other sectors
	};
	const std::vector<BorderLine> lines = chainBorders(edges);
	ASSERT_EQ(lines.size(), 2u);
	EXPECT_EQ(lines[0].right, 1);
	EXPECT_EQ(lines[1].right, 2);
}

TEST(SectorsBorderLines, ABorderAllRoundASectorIsALoop)
{
	// A home in the middle of the map owns the ground within its radius; site 0, at the same spot,
	// wins every tie outside it. So the home is an island.
	MapRules rules = rulesWithCell(40.0f);
	rules.homeRadius = 300.0f;
	const SectorMap map = SectorMap::build({ { { 1000, 1000 }, false }, { { 1000, 1000 }, true } }, 2000, 2000, rules);
	ASSERT_EQ(map.sectorAt({ 1000, 1000 }), 1);
	ASSERT_EQ(map.sectorAt({ 1500, 1000 }), 0);

	const std::vector<BorderLine> lines = chainBorders(map.borders());
	ASSERT_EQ(lines.size(), 1u);
	EXPECT_TRUE(lines[0].closed);
	EXPECT_GT(lines[0].points.size(), 8u);
	EXPECT_FALSE(isNear(lines[0].points.front(), lines[0].points.back()));	// a loop doesn't repeat its start

	const std::vector<BorderLine> smooth = smoothBorders(map, LineRules());
	ASSERT_EQ(smooth.size(), 1u);
	EXPECT_TRUE(smooth[0].closed);
	for (Vec2 point : smooth[0].points)
		EXPECT_NEAR(std::sqrt(distanceSquared(point, { 1000, 1000 })), 300.0f, 25.0f);
}

TEST(SectorsBorderLines, BordersAcrossTheMapAreOpenAndEndAtItsEdges)
{
	const SectorMap map = SectorMap::build({ { { 300, 300 }, false }, { { 1700, 1500 }, false } }, 2000, 2000, rulesWithCell(40.0f));
	const std::vector<BorderLine> lines = chainBorders(map.borders());
	ASSERT_EQ(lines.size(), 1u);
	EXPECT_FALSE(lines[0].closed);
	for (Vec2 end : { lines[0].points.front(), lines[0].points.back() })
		EXPECT_TRUE(end.x == 0 || end.y == 0 || end.x == 2000 || end.y == 2000);
}

TEST(SectorsBorderLines, SmoothingKeepsTheEndsOfAnOpenLine)
{
	const std::vector<Vec2> stairs = { { 0, 0 }, { 40, 0 }, { 40, 40 }, { 80, 40 }, { 80, 80 }, { 120, 80 } };
	for (int passes = 1; passes <= 4; ++passes)
	{
		const std::vector<Vec2> smooth = smoothLine(stairs, false, passes);
		EXPECT_GT(smooth.size(), stairs.size());
		EXPECT_TRUE(isNear(smooth.front(), stairs.front()));
		EXPECT_TRUE(isNear(smooth.back(), stairs.back()));
	}
	EXPECT_EQ(smoothLine(stairs, false, 0).size(), stairs.size());	// 0: the staircase as it is
}

TEST(SectorsBorderLines, SmoothingStraightensAStaircase)
{
	// A staircase of 40-unit steps stands for the diagonal through the middle of its runs.
	std::vector<Vec2> stairs;
	for (int i = 0; i < 10; ++i)
	{
		stairs.push_back({ i * 40.0f, i * 40.0f });
		stairs.push_back({ (i + 1) * 40.0f, i * 40.0f });
	}
	const std::vector<Vec2> smooth = smoothLine(stairs, false, 3);
	for (size_t i = 0; i < smooth.size(); ++i)
	{
		const Vec2 p = smooth[i];
		if (p.x < 60 || p.x > 320)
			continue;	// the ends bend toward the kept end points
		EXPECT_LT(distanceToLine(p, { 20, 0 }, { 60, 40 }), 0.5f) << "point " << i;
	}
}

TEST(SectorsBorderLines, ResamplingSpacesPointsEvenly)
{
	const std::vector<Vec2> line = { { 0, 0 }, { 100, 0 }, { 100, 50 } };
	const std::vector<Vec2> even = resampleLine(line, false, 10.0f);
	ASSERT_EQ(even.size(), 16u);
	EXPECT_TRUE(isNear(even.front(), { 0, 0 }));
	EXPECT_TRUE(isNear(even.back(), { 100, 50 }));
	for (size_t i = 1; i < even.size(); ++i)
		EXPECT_LE(std::sqrt(distanceSquared(even[i - 1], even[i])), 10.01f);

	const std::vector<Vec2> square = { { 0, 0 }, { 100, 0 }, { 100, 100 }, { 0, 100 } };
	EXPECT_EQ(resampleLine(square, true, 10.0f).size(), 40u);	// a loop doesn't repeat its start
}

TEST(SectorsBorderLines, PositiveOffsetsGoLeft)
{
	const std::vector<Vec2> line = { { 0, 0 }, { 10, 0 }, { 20, 0 } };
	const std::vector<Vec2> left = offsetLine(line, false, 2.0f);
	const std::vector<Vec2> right = offsetLine(line, false, -2.0f);
	for (size_t i = 0; i < line.size(); ++i)
	{
		EXPECT_TRUE(isNear(left[i], { line[i].x, 2.0f }));
		EXPECT_TRUE(isNear(right[i], { line[i].x, -2.0f }));
	}

	// At a right-angle corner both segments move by the full distance.
	const std::vector<Vec2> corner = offsetLine({ { 0, 0 }, { 10, 0 }, { 10, 10 } }, false, 1.0f);
	EXPECT_TRUE(isNear(corner[1], { 9, 1 }));
}

TEST(SectorsBorderLines, InsetLinesStayInsideTheirSector)
{
	const std::vector<Site> sites = {
		{ { 600, 700 }, true },
		{ { 3400, 3300 }, true },
		{ { 2000, 2000 }, false },
		{ { 900, 3100 }, false },
		{ { 3000, 800 }, false },
		{ { 1900, 900 }, false },
		{ { 2200, 3200 }, false },
	};
	MapRules rules = rulesWithCell(40.0f);
	rules.homeRadius = 450.0f;
	const SectorMap map = SectorMap::build(sites, 4000, 4000, rules);
	const std::vector<BorderLine> lines = smoothBorders(map, LineRules());
	ASSERT_GE(lines.size(), 8u);

	int checked = 0;
	for (const BorderLine &line : lines)
	{
		for (int side : { line.left, line.right })
		{
			const std::vector<LinePiece> pieces = insetLine(line, side, 30.0f, map);
			EXPECT_FALSE(pieces.empty());
			for (const LinePiece &piece : pieces)
			{
				for (Vec2 point : piece.points)
				{
					EXPECT_EQ(map.sectorAt(point), side) << "(" << point.x << ", " << point.y << ")";
					++checked;
				}
			}
		}
	}
	EXPECT_GT(checked, 500);
}

TEST(SectorsBorderLines, InsetsAreCutWhereTheyCrossIntoAThirdSector)
{
	// Three sectors meeting in a Y: insets running past the meeting point are cut.
	const std::vector<Site> sites = { { { 1000, 400 }, false }, { { 400, 1600 }, false }, { { 1600, 1600 }, false } };
	const SectorMap map = SectorMap::build(sites, 2000, 2000, rulesWithCell(40.0f));
	for (const BorderLine &line : smoothBorders(map, LineRules()))
	{
		for (int side : { line.left, line.right })
		{
			for (const LinePiece &piece : insetLine(line, side, 60.0f, map))
			{
				for (Vec2 point : piece.points)
				{
					const int sector = map.sectorAt(point);
					EXPECT_TRUE(sector == line.left || sector == line.right);
				}
			}
		}
	}
}

TEST(SectorsBorderLines, ARegionOfSeveralSectorsHasOneConnectedOutline)
{
	// Four sectors in a row across the map; the middle two are one region, so their outline is one
	// loop round both, with no line between them.
	const std::vector<Site> sites = { { { 250, 500 }, false }, { { 750, 500 }, false }, { { 1250, 500 }, false }, { { 1750, 500 }, false } };
	const SectorMap map = SectorMap::build(sites, 2000, 1000, rulesWithCell(40.0f));
	const std::vector<RegionOutline> outlines = outlineRegions(map, { 0, 1, 1, 2 }, LineRules());

	int middle = 0;
	for (const RegionOutline &outline : outlines)
	{
		if (outline.region != 1)
			continue;
		++middle;
		EXPECT_FALSE(outline.closed);	// both of its borders reach the map's edges, which have no line
		for (Vec2 point : outline.points)
			EXPECT_FALSE(point.x > 900 && point.x < 1100 && point.y > 50 && point.y < 950);	// nothing at x = 1000
	}
	EXPECT_EQ(middle, 2);	// its west border and its east border
}

TEST(SectorsBorderLines, OutlinesRunOnRoundCornersWhereThreeSectorsMeet)
{
	// Three sectors meeting in a Y: each one's outline is one line through the meeting point.
	const std::vector<Site> sites = { { { 1000, 400 }, false }, { { 400, 1600 }, false }, { { 1600, 1600 }, false } };
	const SectorMap map = SectorMap::build(sites, 2000, 2000, rulesWithCell(40.0f));
	const std::vector<RegionOutline> outlines = outlineRegions(map, { 0, 1, 2 }, LineRules());
	ASSERT_EQ(outlines.size(), 3u);
	for (const RegionOutline &outline : outlines)
	{
		EXPECT_FALSE(outline.closed);
		for (Vec2 end : { outline.points.front(), outline.points.back() })
			EXPECT_TRUE(end.x == 0 || end.y == 0 || end.x == 2000 || end.y == 2000);
	}
}

TEST(SectorsBorderLines, InsetOutlinesStayInsideTheirRegion)
{
	const std::vector<Site> sites = {
		{ { 600, 700 }, true },
		{ { 3400, 3300 }, true },
		{ { 2000, 2000 }, false },
		{ { 900, 3100 }, false },
		{ { 3000, 800 }, false },
		{ { 1900, 900 }, false },
		{ { 2200, 3200 }, false },
	};
	MapRules rules = rulesWithCell(40.0f);
	rules.homeRadius = 450.0f;
	const SectorMap map = SectorMap::build(sites, 4000, 4000, rules);
	const std::vector<int> regionOf = { 0, 1, 2, 0, 1, 0, 2 };	// players 0, 1 and 2 hold a few sectors each

	int checked = 0;
	for (const RegionOutline &outline : outlineRegions(map, regionOf, LineRules()))
	{
		for (const LinePiece &piece : insetOutline(outline, 30.0f, map))
		{
			for (Vec2 point : piece.points)
			{
				EXPECT_EQ(regionOf[map.sectorAt(point)], outline.region) << "(" << point.x << ", " << point.y << ")";
				++checked;
			}
		}
	}
	EXPECT_GT(checked, 500);
}

TEST(SectorsBorderLines, AnInsetLoopCutAtTheMapEdgeStaysOnePiece)
{
	// A closed outline poking off the map: what is left is one open piece, not split where the loop starts.
	RegionOutline outline;
	outline.region = 0;
	outline.closed = true;
	for (int i = 0; i < 36; ++i)
	{
		const float angle = i * 10.0f * 3.14159265f / 180.0f;
		outline.points.push_back({ 100.0f + 150.0f * std::cos(angle), 500.0f + 150.0f * std::sin(angle) });
	}
	const SectorMap map = SectorMap::build({ { { 500, 500 }, false } }, 1000, 1000, rulesWithCell(40.0f));
	const std::vector<LinePiece> pieces = insetOutline(outline, -10.0f, map);
	ASSERT_EQ(pieces.size(), 1u);
	EXPECT_FALSE(pieces[0].closed);
}

TEST(SectorsBorderLines, BandsNarrowOnlyWhereTheLineTurnsTightlyTowardThem)
{
	// A straight run, a tight left turn (toward the band) of radius 20, and another straight run.
	std::vector<Vec2> line;
	for (int i = 0; i <= 10; ++i)
		line.push_back({ i * 12.0f - 120.0f, -20.0f });
	for (int i = 1; i < 6; ++i)
	{
		const float angle = -3.14159265f / 2 + i * 3.14159265f / 6;
		line.push_back({ 20.0f * std::cos(angle), 20.0f * std::sin(angle) });
	}
	for (int i = 0; i <= 10; ++i)
		line.push_back({ -i * 12.0f, 20.0f });

	const std::vector<float> depths = bandDepths(line, false, 40.0f);
	ASSERT_EQ(depths.size(), line.size());
	EXPECT_FLOAT_EQ(depths.front(), 40.0f);
	EXPECT_FLOAT_EQ(depths.back(), 40.0f);
	EXPECT_LT(depths[13], 20.0f);	// in the turn: no deeper than its radius
	for (float depth : depths)
		EXPECT_GT(depth, 0.0f);

	// Turning the other way (away from the band) needs no narrowing.
	std::vector<Vec2> reversed(line.rbegin(), line.rend());
	for (float depth : bandDepths(reversed, false, 40.0f))
		EXPECT_FLOAT_EQ(depth, 40.0f);
}
