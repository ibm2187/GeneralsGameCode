// CoH Sectors: sectors drawn on the ground. Each border is a pair of lines, one inside each sector in
// its owner's color. Each point has a ring in its owner's color with the capture progress as an inner
// arc; a contested point's ring blinks, and a cut-off sector's ring is broken.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/Display.h"
#include "GameClient/View.h"
#include "GameLogic/TerrainLogic.h"

#include "Features/Sectors/Engine/Config.h"
#include "Features/Sectors/Engine/Internal.h"
#include "Features/Sectors/Engine/SectorPointUpdate.h"
#include "Features/Sectors/Engine/World.h"
#include "Features/Sectors/UI/Colors.h"

#include <cmath>

namespace CoH::Sectors
{

namespace
{

const Real BORDER_INSET = 4.0f;		///< world units each side's line sits inside its sector
const Real LIFT = 1.5f;				///< above the ground, so lines don't sink into it
const Int RING_SEGMENTS = 40;

Coord3D onGround(Vec2 point)
{
	Coord3D world = { point.x, point.y, 0.0f };
	world.z = TheTerrainLogic->getGroundHeight(world.x, world.y) + LIFT;
	return world;
}

void drawWorldLine(Vec2 a, Vec2 b, Real width, Color color)
{
	const Coord3D worldA = onGround(a);
	const Coord3D worldB = onGround(b);
	ICoord2D screenA, screenB;
	const View::WorldToScreenReturn ra = TheTacticalView->worldToScreenTriReturn(&worldA, &screenA);
	const View::WorldToScreenReturn rb = TheTacticalView->worldToScreenTriReturn(&worldB, &screenB);
	if (ra == View::WTS_INVALID || rb == View::WTS_INVALID)
		return;
	if (ra != View::WTS_INSIDE_FRUSTUM && rb != View::WTS_INSIDE_FRUSTUM)
		return;
	TheDisplay->drawLine(screenA.x, screenA.y, screenB.x, screenB.y, width, color);
}

void drawBorder(const World &w, const BorderEdge &edge)
{
	const PlayerId left = w.ownersAtRefresh()[edge.left];
	const PlayerId right = w.ownersAtRefresh()[edge.right];
	const UnsignedByte alpha = left == right ? 90 : 170;

	const Real dx = edge.b.x - edge.a.x;
	const Real dy = edge.b.y - edge.a.y;
	const Real length = std::sqrt(dx * dx + dy * dy);
	if (length <= 0.0f)
		return;

	const Vec2 leftNormal = { -dy / length * BORDER_INSET, dx / length * BORDER_INSET };
	const Int pieces = std::max(1, (Int)std::ceil(length / (w.map().cellSize() * 2.0f)));
	for (Int i = 0; i < pieces; ++i)
	{
		const Real t0 = (Real)i / pieces;
		const Real t1 = (Real)(i + 1) / pieces;
		const Vec2 a = { edge.a.x + dx * t0, edge.a.y + dy * t0 };
		const Vec2 b = { edge.a.x + dx * t1, edge.a.y + dy * t1 };
		drawWorldLine({ a.x + leftNormal.x, a.y + leftNormal.y }, { b.x + leftNormal.x, b.y + leftNormal.y }, 2.0f,
			ownerColor(left, alpha));
		drawWorldLine({ a.x - leftNormal.x, a.y - leftNormal.y }, { b.x - leftNormal.x, b.y - leftNormal.y }, 2.0f,
			ownerColor(right, alpha));
	}
}

Vec2 onCircle(Vec2 center, Real radius, Real turn)
{
	const Real angle = turn * 2.0f * PI + PI / 2.0f;	// from the top, clockwise on screen
	return { center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius };
}

// From the top of the circle, 'fraction' of the way round.
void drawArc(Vec2 center, Real radius, Real fraction, Real width, Color color, bool broken)
{
	const Int segments = (Int)std::ceil(RING_SEGMENTS * fraction);
	for (Int i = 0; i < segments; ++i)
	{
		if (broken && i % 2 == 1)
			continue;
		const Real from = (Real)i / RING_SEGMENTS;
		const Real to = std::min(fraction, (Real)(i + 1) / RING_SEGMENTS);
		drawWorldLine(onCircle(center, radius, -from), onCircle(center, radius, -to), width, color);
	}
}

void drawPoint(const World &w, int index)
{
	const SectorPointUpdate *point = w.point(index);
	if (point == nullptr)
		return;

	const PointState &state = point->state();
	const Vec2 center = w.sector(index).position;
	const Real radius = config().captureRadius;
	const bool contested = w.occupation(index).kind == Occupation::Kind::Contested;
	const bool cutOff = w.isCutOff(index);

	Color ring = ownerColor(state.owner, 220);
	if (contested && blinkOn())
		ring = GameMakeColor(255, 255, 255, 230);
	drawArc(center, radius, 1.0f, 2.0f, ring, cutOff);

	// The capture progress: the owner's hold while owned, else whoever is taking it.
	const bool full = state.owner != NoPlayer && state.progress >= 1.0f;
	if (!full && state.progress > 0.0f)
		drawArc(center, radius * 0.85f, state.progress, 4.0f, ownerColor(state.progressSide, 255), false);

	if (w.sector(index).kind == SectorKind::HighValue)
		drawArc(center, radius * 0.7f, 1.0f, 1.0f, ownerColor(state.owner, 160), false);
}

} // namespace

void drawGround()
{
	const World &w = world();
	if (!w.hasSectors() || TheTacticalView == nullptr || TheDisplay == nullptr || TheTerrainLogic == nullptr)
		return;

	for (const BorderEdge &edge : w.map().borders())
		drawBorder(w, edge);
	for (int i = 0; i < w.sectorCount(); ++i)
		drawPoint(w, i);
}

} // namespace CoH::Sectors
