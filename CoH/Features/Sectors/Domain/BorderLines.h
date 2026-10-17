// CoH Sectors: the borders as smooth lines, for drawing. The grid's straight border edges are chained
// into one line per stretch of border between the same two sectors (a loop where a border closes),
// rounded, and evenly spaced; each side then gets lines offset into its own sector. Pure geometry for
// the UI: nothing here changes which sector a position is in.

#pragma once

#include "SectorMap.h"

#include <vector>

namespace CoH::Sectors
{

// A continuous border between two sectors. Seen along the points, 'left' is on the left.
struct BorderLine
{
	std::vector<Vec2> points;
	int left = -1;
	int right = -1;
	bool closed = false;	///< a loop: the last point joins the first
};

struct LineRules
{
	int smoothing = 3;		///< rounding passes; 0 keeps the grid's staircase
	float spacing = 12.0f;	///< world units between the points of a smoothed line
};

// Joins edges that meet end to end into lines, one per stretch between the same pair of sectors. The
// lower sector number is always on the left.
std::vector<BorderLine> chainBorders(const std::vector<BorderEdge> &edges);

// Rounds a line: first it goes through the middle of each straight run, which turns the grid's
// staircase back into the slanted line it stands for, then each pass cuts every corner (Chaikin). The
// ends of an open line stay where they are, so lines that meet still meet. 0 passes changes nothing.
std::vector<Vec2> smoothLine(const std::vector<Vec2> &points, bool closed, int passes);

// The same line with points evenly spaced, at most 'spacing' apart. An open line keeps its ends.
std::vector<Vec2> resampleLine(const std::vector<Vec2> &points, bool closed, float spacing);

// The line moved sideways: to its left for a positive distance, to its right for a negative one.
std::vector<Vec2> offsetLine(const std::vector<Vec2> &points, bool closed, float distance);

// The same, each point moved by its own distance.
std::vector<Vec2> offsetLine(const std::vector<Vec2> &points, bool closed, const std::vector<float> &distances);

// How far a band can reach to the left of each point without folding over itself: 'depth', except
// where the line curves to the left more tightly than that, where it narrows to fit the curve.
// Changes are eased along the line, so the band's inner edge stays smooth.
std::vector<float> bandDepths(const std::vector<Vec2> &points, bool closed, float depth);

// Every border of a map, chained, smoothed and resampled.
std::vector<BorderLine> smoothBorders(const SectorMap &map, const LineRules &rules);

struct LinePiece
{
	std::vector<Vec2> points;
	bool closed = false;
};

// The edge of a region: ground made of whole sectors, such as everything one player holds. The
// region is on the left of the points.
struct RegionOutline
{
	std::vector<Vec2> points;
	int region = -1;
	bool closed = false;
};

// The outlines of the regions the sectors form, given each sector's region, chained, smoothed and
// resampled. A region's outline runs on round the corners where three sectors meet, so each region
// has one connected line (open where it reaches the map's edge, which has no line).
std::vector<RegionOutline> outlineRegions(const SectorMap &map, const std::vector<int> &regionOf, const LineRules &rules);

// The outline moved 'inset' world units into its region; cut where it leaves the map.
std::vector<LinePiece> insetOutline(const RegionOutline &outline, float inset, const SectorMap &map);

// The line moved 'inset' world units into sector 'side' (one of line.left and line.right). Where the
// moved line strays into a third sector, as it does near a corner where three sectors meet, or off the
// map, it is cut, so what is left lies in its own sector or on the border it follows.
std::vector<LinePiece> insetLine(const BorderLine &line, int side, float inset, const SectorMap &map);

} // namespace CoH::Sectors
