// CoH Sectors: sectors on the minimap. Each owned sector is lightly tinted in its owner's color, the
// smooth borders are drawn in the colors of the sectors on each side (a faint line between two sectors
// of one owner), each point is a dot in its owner's color, and a cut-off sector's dot is hollow.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/Display.h"
#include "GameLogic/TerrainLogic.h"

#include "Features/Sectors/Engine/Internal.h"
#include "Features/Sectors/Engine/World.h"
#include "Features/Sectors/UI/BorderGeometry.h"
#include "Features/Sectors/UI/Colors.h"

#include <cmath>

namespace CoH::Sectors
{

namespace
{

const Int DOT_SIZE = 5;
const UnsignedByte TINT_ALPHA = 55;

struct RadarArea
{
	Int x, y, width, height;
	Real worldWidth, worldHeight;

	ICoord2D toPixel(Vec2 world) const
	{
		// The radar shows +y up, like the world seen from above.
		return { x + (Int)(world.x / worldWidth * width), y + height - (Int)(world.y / worldHeight * height) };
	}

	Vec2 toWorld(Int column, Int row) const
	{
		return { (column + 0.5f) / width * worldWidth, (height - row - 0.5f) / height * worldHeight };
	}
};

// A run of radar pixels on one row that are all in one sector.
struct Run
{
	Int column, row, length;
	int sector;
};

// The tint's runs, worked out again only when the sectors or the radar's size change.
const std::vector<Run> &tintRuns(const RadarArea &area, const World &w)
{
	static std::vector<Run> runs;
	static int buildNumber = -1;
	static Int width = 0, height = 0;
	if (buildNumber == w.buildNumber() && width == area.width && height == area.height)
		return runs;

	buildNumber = w.buildNumber();
	width = area.width;
	height = area.height;
	runs.clear();
	for (Int row = 0; row < area.height; ++row)
	{
		for (Int column = 0; column < area.width;)
		{
			const int sector = w.map().sectorAt(area.toWorld(column, row));
			Int end = column + 1;
			while (end < area.width && w.map().sectorAt(area.toWorld(end, row)) == sector)
				++end;
			runs.push_back({ column, row, end - column, sector });
			column = end;
		}
	}
	return runs;
}

void drawTint(const RadarArea &area, const World &w)
{
	const std::vector<PlayerId> &owners = w.ownersAtRefresh();
	for (const Run &run : tintRuns(area, w))
	{
		const PlayerId owner = owners[run.sector];
		if (owner != NoPlayer)
			TheDisplay->drawFillRect(area.x + run.column, area.y + run.row, run.length, 1, ownerColor(owner, TINT_ALPHA));
	}
}

void drawBorder(const RadarArea &area, const World &w, const BorderLine &line)
{
	const PlayerId left = w.ownersAtRefresh()[line.left];
	const PlayerId right = w.ownersAtRefresh()[line.right];
	const bool shared = left == right;
	const Color leftColor = ownerColor(left, shared ? 110 : 230);
	const Color rightColor = ownerColor(right, 230);

	const size_t n = line.points.size();
	const size_t segments = line.closed ? n : n - 1;
	ICoord2D from = area.toPixel(line.points[0]);
	for (size_t i = 0; i < segments; ++i)
	{
		const Vec2 a = line.points[i];
		const Vec2 b = line.points[(i + 1) % n];
		const ICoord2D to = area.toPixel(b);
		if (to.x == from.x && to.y == from.y)
			continue;	// shorter than a pixel: carry on from the same pixel

		if (shared)
		{
			TheDisplay->drawLine(from.x, from.y, to.x, to.y, 1.0f, leftColor);
		}
		else
		{
			// Each side's color one pixel toward its own side. Seen from above with +y up, the world's
			// left of a -> b is the screen's (-dy, -dx), rounded here to a whole pixel step.
			const Real dx = b.x - a.x;
			const Real dy = b.y - a.y;
			const ICoord2D toLeft = std::fabs(dx) >= std::fabs(dy) ? ICoord2D{ 0, dx > 0 ? -1 : 1 } : ICoord2D{ dy > 0 ? -1 : 1, 0 };
			TheDisplay->drawLine(from.x + toLeft.x, from.y + toLeft.y, to.x + toLeft.x, to.y + toLeft.y, 1.0f, leftColor);
			TheDisplay->drawLine(from.x, from.y, to.x, to.y, 1.0f, rightColor);
		}
		from = to;
	}
}

void drawPoint(const RadarArea &area, const World &w, int index)
{
	const ICoord2D center = area.toPixel(w.sector(index).position);
	const Color color = ownerColor(w.ownersAtRefresh()[index], 255);
	const Int left = center.x - DOT_SIZE / 2;
	const Int top = center.y - DOT_SIZE / 2;

	TheDisplay->drawFillRect(left - 1, top - 1, DOT_SIZE + 2, DOT_SIZE + 2, GameMakeColor(0, 0, 0, 255));
	if (w.isCutOff(index))
		TheDisplay->drawOpenRect(left, top, DOT_SIZE, DOT_SIZE, 1.0f, color);
	else
		TheDisplay->drawFillRect(left, top, DOT_SIZE, DOT_SIZE, color);
}

} // namespace

void drawRadar(int x, int y, int width, int height)
{
	const World &w = world();
	if (!w.hasSectors() || TheDisplay == nullptr || TheTerrainLogic == nullptr || width <= 0 || height <= 0)
		return;

	Region3D extent;
	TheTerrainLogic->getExtent(&extent);
	if (extent.hi.x <= 0.0f || extent.hi.y <= 0.0f)
		return;

	const RadarArea area = { x, y, width, height, extent.hi.x, extent.hi.y };
	drawTint(area, w);
	for (const BorderLine &line : sectorGeometry(w).lines)
	{
		if (line.points.size() >= 2)
			drawBorder(area, w, line);
	}
	for (int i = 0; i < w.sectorCount(); ++i)
		drawPoint(area, w, i);
}

} // namespace CoH::Sectors
