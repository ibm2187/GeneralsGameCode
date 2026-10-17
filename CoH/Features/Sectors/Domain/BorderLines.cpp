// CoH Sectors: see BorderLines.h.

#include "BorderLines.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <utility>

namespace CoH::Sectors
{

namespace
{

using PointKey = std::pair<float, float>;

PointKey keyOf(Vec2 point)
{
	return { point.x, point.y };
}

bool samePoint(Vec2 a, Vec2 b)
{
	return a.x == b.x && a.y == b.y;
}

Vec2 lerp(Vec2 a, Vec2 b, float t)
{
	return { a.x + (b.x - a.x) * t, a.y + (b.y - a.y) * t };
}

float length(Vec2 a, Vec2 b)
{
	return std::sqrt(distanceSquared(a, b));
}

// The unit normal to the left of a -> b, or zero if the points are the same.
Vec2 leftNormal(Vec2 a, Vec2 b)
{
	const float d = length(a, b);
	if (d <= 0.0f)
		return {};
	return { -(b.y - a.y) / d, (b.x - a.x) / d };
}

// Edges joined end to end, each line going the way its edges go.
std::vector<LinePiece> chainEdges(const std::vector<BorderEdge> &edges)
{
	std::multimap<PointKey, size_t> startingAt;
	std::map<PointKey, int> balance;	///< edges leaving a point minus edges arriving
	for (size_t i = 0; i < edges.size(); ++i)
	{
		startingAt.insert({ keyOf(edges[i].a), i });
		++balance[keyOf(edges[i].a)];
		--balance[keyOf(edges[i].b)];
	}

	std::vector<LinePiece> lines;
	std::vector<bool> used(edges.size(), false);
	const auto follow = [&](size_t first)
	{
		LinePiece line;
		line.points = { edges[first].a, edges[first].b };
		used[first] = true;

		for (;;)
		{
			const auto range = startingAt.equal_range(keyOf(line.points.back()));
			auto next = range.first;
			while (next != range.second && used[next->second])
				++next;
			if (next == range.second)
				break;
			used[next->second] = true;
			line.points.push_back(edges[next->second].b);
		}

		if (line.points.size() > 2 && samePoint(line.points.front(), line.points.back()))
		{
			line.points.pop_back();
			line.closed = true;
		}
		lines.push_back(std::move(line));
	};

	// Open lines first, from the points where more edges leave than arrive; what is left are loops.
	for (size_t i = 0; i < edges.size(); ++i)
	{
		if (!used[i] && balance[keyOf(edges[i].a)] > 0)
			follow(i);
	}
	for (size_t i = 0; i < edges.size(); ++i)
	{
		if (!used[i])
			follow(i);
	}
	return lines;
}

// Without the points in the middle of a straight run, so a run split where the sector across it
// changes counts as one run when smoothing.
std::vector<Vec2> withoutStraightPoints(const std::vector<Vec2> &points, bool closed)
{
	const size_t n = points.size();
	if (n < 3)
		return points;
	std::vector<Vec2> result;
	for (size_t i = 0; i < n; ++i)
	{
		const bool end = !closed && (i == 0 || i + 1 == n);
		const Vec2 before = points[(i + n - 1) % n];
		const Vec2 after = points[(i + 1) % n];
		const float cross = (points[i].x - before.x) * (after.y - points[i].y) - (points[i].y - before.y) * (after.x - points[i].x);
		if (end || cross != 0.0f)
			result.push_back(points[i]);
	}
	return result.size() >= (closed ? 3u : 2u) ? result : points;
}

std::vector<Vec2> midpoints(const std::vector<Vec2> &points, bool closed)
{
	const size_t n = points.size();
	std::vector<Vec2> result;
	if (closed)
	{
		for (size_t i = 0; i < n; ++i)
			result.push_back(lerp(points[i], points[(i + 1) % n], 0.5f));
		return result;
	}

	result.push_back(points.front());
	for (size_t i = 0; i + 1 < n; ++i)
		result.push_back(lerp(points[i], points[i + 1], 0.5f));
	result.push_back(points.back());
	return result;
}

std::vector<Vec2> cutCorners(const std::vector<Vec2> &points, bool closed)
{
	const size_t n = points.size();
	std::vector<Vec2> result;
	if (closed)
	{
		for (size_t i = 0; i < n; ++i)
		{
			result.push_back(lerp(points[i], points[(i + 1) % n], 0.25f));
			result.push_back(lerp(points[i], points[(i + 1) % n], 0.75f));
		}
		return result;
	}

	result.push_back(points.front());
	for (size_t i = 0; i + 1 < n; ++i)
	{
		if (i > 0)
			result.push_back(lerp(points[i], points[i + 1], 0.25f));
		if (i + 2 < n)
			result.push_back(lerp(points[i], points[i + 1], 0.75f));
	}
	result.push_back(points.back());
	return result;
}

bool isOnMap(Vec2 point, const SectorMap &map)
{
	return point.x >= 0.0f && point.y >= 0.0f && point.x <= map.gridWidth() * map.cellSize()
		&& point.y <= map.gridHeight() * map.cellSize();
}

// The runs of kept points, as pieces of at least two points. A loop is walked from a dropped point,
// so no piece is split where the loop starts.
std::vector<LinePiece> keptPieces(const std::vector<Vec2> &points, bool closed, const std::vector<bool> &keep)
{
	const size_t n = points.size();
	std::vector<LinePiece> pieces;
	if (n < 2)
		return pieces;

	const size_t firstDropped = std::find(keep.begin(), keep.end(), false) - keep.begin();
	if (firstDropped == n)
	{
		pieces.push_back({ points, closed });
		return pieces;
	}

	const size_t start = closed ? firstDropped : 0;
	LinePiece piece;
	for (size_t k = 0; k <= n; ++k)
	{
		const size_t i = (start + k) % n;
		if (k < n && keep[i])
		{
			piece.points.push_back(points[i]);
			continue;
		}
		if (piece.points.size() >= 2)
			pieces.push_back(piece);
		piece.points.clear();
	}
	return pieces;
}

} // namespace

std::vector<BorderLine> chainBorders(const std::vector<BorderEdge> &edges)
{
	// Each pair's edges, turned round where needed so the lower sector is on the left.
	std::map<std::pair<int, int>, std::vector<BorderEdge>> pairs;
	for (BorderEdge edge : edges)
	{
		if (edge.left > edge.right)
		{
			std::swap(edge.a, edge.b);
			std::swap(edge.left, edge.right);
		}
		pairs[{ edge.left, edge.right }].push_back(edge);
	}

	std::vector<BorderLine> lines;
	for (const auto &pair : pairs)
	{
		for (LinePiece &piece : chainEdges(pair.second))
			lines.push_back({ std::move(piece.points), pair.first.first, pair.first.second, piece.closed });
	}
	return lines;
}

std::vector<Vec2> smoothLine(const std::vector<Vec2> &points, bool closed, int passes)
{
	if (passes <= 0 || points.size() < (closed ? 3u : 2u))
		return points;

	std::vector<Vec2> result = midpoints(withoutStraightPoints(points, closed), closed);
	for (int i = 0; i < passes; ++i)
		result = cutCorners(result, closed);
	return result;
}

std::vector<Vec2> resampleLine(const std::vector<Vec2> &points, bool closed, float spacing)
{
	const size_t n = points.size();
	if (n < 2 || spacing <= 0.0f)
		return points;

	const size_t segments = closed ? n : n - 1;
	float total = 0.0f;
	for (size_t i = 0; i < segments; ++i)
		total += length(points[i], points[(i + 1) % n]);
	if (total <= 0.0f)
		return { points.front() };

	const int steps = std::max(closed ? 3 : 1, (int)std::ceil(total / spacing));
	const float step = total / steps;

	std::vector<Vec2> result;
	size_t segment = 0;
	float segmentStart = 0.0f;	///< distance along the line where the current segment starts
	const int last = closed ? steps - 1 : steps;
	for (int i = 0; i <= last; ++i)
	{
		const float at = i * step;
		while (segment + 1 < segments && segmentStart + length(points[segment], points[(segment + 1) % n]) < at)
		{
			segmentStart += length(points[segment], points[(segment + 1) % n]);
			++segment;
		}
		const Vec2 a = points[segment];
		const Vec2 b = points[(segment + 1) % n];
		const float d = length(a, b);
		result.push_back(d > 0.0f ? lerp(a, b, std::clamp((at - segmentStart) / d, 0.0f, 1.0f)) : a);
	}
	if (!closed)
		result.back() = points.back();
	return result;
}

std::vector<Vec2> offsetLine(const std::vector<Vec2> &points, bool closed, float distance)
{
	return offsetLine(points, closed, std::vector<float>(points.size(), distance));
}

std::vector<float> bandDepths(const std::vector<Vec2> &points, bool closed, float depth)
{
	const int n = (int)points.size();
	std::vector<float> limit(n, depth);
	if (n < 3 || depth <= 0.0f)
		return limit;

	// The radius of the circle through a point and the points a few steps either side, where the
	// line turns left (toward the band).
	const int reach = std::max(1, std::min(3, (n - 1) / 2));
	const auto at = [&](int i) -> Vec2 { return points[closed ? (i % n + n) % n : std::clamp(i, 0, n - 1)]; };
	for (int i = 0; i < n; ++i)
	{
		const Vec2 a = at(i - reach);
		const Vec2 b = at(i);
		const Vec2 c = at(i + reach);
		const float cross = (b.x - a.x) * (c.y - b.y) - (b.y - a.y) * (c.x - b.x);
		if (cross <= 0.0f)
			continue;
		const float radius = length(a, b) * length(b, c) * length(a, c) / (2.0f * cross);
		limit[i] = std::min(depth, 0.8f * radius);
	}

	// Narrow the points either side of a tight turn as well, then ease the changes.
	std::vector<float> widened(n);
	for (int i = 0; i < n; ++i)
	{
		widened[i] = limit[i];
		for (int k = -reach; k <= reach; ++k)
		{
			const int j = closed ? ((i + k) % n + n) % n : std::clamp(i + k, 0, n - 1);
			widened[i] = std::min(widened[i], limit[j]);
		}
	}
	std::vector<float> eased(n);
	for (int i = 0; i < n; ++i)
	{
		float sum = 0.0f;
		for (int k = -reach; k <= reach; ++k)
			sum += widened[closed ? ((i + k) % n + n) % n : std::clamp(i + k, 0, n - 1)];
		eased[i] = sum / (2 * reach + 1);	// every value averaged is within reach of i, so no more than limit[i]
	}
	return eased;
}

std::vector<Vec2> offsetLine(const std::vector<Vec2> &points, bool closed, const std::vector<float> &distances)
{
	const size_t n = points.size();
	if (n < 2)
		return points;

	std::vector<Vec2> result;
	result.reserve(n);
	for (size_t i = 0; i < n; ++i)
	{
		const bool hasBefore = closed || i > 0;
		const bool hasAfter = closed || i + 1 < n;
		const Vec2 before = hasBefore ? leftNormal(points[(i + n - 1) % n], points[i]) : Vec2{};
		const Vec2 after = hasAfter ? leftNormal(points[i], points[(i + 1) % n]) : Vec2{};

		// The mitred normal: the average of the two segments' normals, lengthened so both segments
		// move by the full distance, but not more than twice, so sharp corners don't spike.
		Vec2 normal = { before.x + after.x, before.y + after.y };
		const float size = std::sqrt(normal.x * normal.x + normal.y * normal.y);
		if (size > 0.0f)
		{
			normal = { normal.x / size, normal.y / size };
			const Vec2 segmentNormal = hasAfter ? after : before;
			const float cosine = std::max(0.5f, normal.x * segmentNormal.x + normal.y * segmentNormal.y);
			normal = { normal.x / cosine, normal.y / cosine };
		}
		result.push_back({ points[i].x + normal.x * distances[i], points[i].y + normal.y * distances[i] });
	}
	return result;
}

std::vector<BorderLine> smoothBorders(const SectorMap &map, const LineRules &rules)
{
	std::vector<BorderLine> lines = chainBorders(map.borders());
	for (BorderLine &line : lines)
	{
		line.points = smoothLine(line.points, line.closed, rules.smoothing);
		line.points = resampleLine(line.points, line.closed, rules.spacing);
	}
	return lines;
}

std::vector<RegionOutline> outlineRegions(const SectorMap &map, const std::vector<int> &regionOf, const LineRules &rules)
{
	// Each region's edges, turned so the region is on the left.
	std::map<int, std::vector<BorderEdge>> regions;
	for (const BorderEdge &edge : map.borders())
	{
		const int left = regionOf[edge.left];
		const int right = regionOf[edge.right];
		if (left == right)
			continue;
		regions[left].push_back(edge);
		regions[right].push_back({ edge.b, edge.a, edge.right, edge.left });
	}

	std::vector<RegionOutline> outlines;
	for (const auto &region : regions)
	{
		for (const LinePiece &piece : chainEdges(region.second))
		{
			RegionOutline outline;
			outline.region = region.first;
			outline.closed = piece.closed;
			outline.points = smoothLine(piece.points, piece.closed, rules.smoothing);
			outline.points = resampleLine(outline.points, piece.closed, rules.spacing);
			outlines.push_back(std::move(outline));
		}
	}
	return outlines;
}

std::vector<LinePiece> insetOutline(const RegionOutline &outline, float inset, const SectorMap &map)
{
	const std::vector<Vec2> moved = offsetLine(outline.points, outline.closed, inset);
	std::vector<bool> keep(moved.size());
	for (size_t i = 0; i < moved.size(); ++i)
		keep[i] = isOnMap(moved[i], map);
	return keptPieces(moved, outline.closed, keep);
}

std::vector<LinePiece> insetLine(const BorderLine &line, int side, float inset, const SectorMap &map)
{
	const float distance = side == line.left ? inset : -inset;
	const std::vector<Vec2> moved = offsetLine(line.points, line.closed, distance);
	std::vector<bool> keep(moved.size());
	for (size_t i = 0; i < moved.size(); ++i)
	{
		const int sector = map.sectorAt(moved[i]);
		keep[i] = isOnMap(moved[i], map) && (sector == line.left || sector == line.right);
	}
	return keptPieces(moved, line.closed, keep);
}

} // namespace CoH::Sectors
