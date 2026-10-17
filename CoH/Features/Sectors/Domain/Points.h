// CoH Sectors: which capture points a map gets. Every supply source and tech building on the map is a
// source; sources close together become one point, and sources close to a start position join that
// player's home point.

#pragma once

#include "Types.h"

#include <vector>

namespace CoH::Sectors
{

struct Source
{
	Vec2 position;
	SectorKind kind = SectorKind::Supply;	///< Supply or HighValue
	int id = 0;								///< the caller's handle for the source, such as an object ID
};

struct Start
{
	Vec2 position;
	PlayerId player = NoPlayer;
};

struct PointPlan
{
	Vec2 position;
	SectorKind kind = SectorKind::Supply;
	PlayerId homePlayer = NoPlayer;	///< the player a Home point belongs to
	std::vector<int> members;		///< ids of the sources the point is made of
};

struct PointRules
{
	float mergeDistance = 300.0f;		///< sources this close (in a chain) form one point
	float homeMergeDistance = 450.0f;	///< a point this close to a start joins its home point
};

// One Home point per start, in the order of the starts, then the other points in the order of their
// first source. A point made of several sources sits at their middle; it is HighValue when any of
// them is.
std::vector<PointPlan> planPoints(const std::vector<Source> &sources, const std::vector<Start> &starts,
	const PointRules &rules);

} // namespace CoH::Sectors
