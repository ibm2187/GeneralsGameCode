// CoH Sectors: sectors on the minimap. Borders are drawn in the colors of the sectors on each side,
// each point is a dot in its owner's color, and a cut-off sector's dot is hollow.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/Display.h"
#include "GameLogic/TerrainLogic.h"

#include "Features/Sectors/Engine/Internal.h"
#include "Features/Sectors/Engine/World.h"
#include "Features/Sectors/UI/Colors.h"

namespace CoH::Sectors
{

namespace
{

const Int DOT_SIZE = 5;

struct RadarArea
{
	Int x, y, width, height;
	Real worldWidth, worldHeight;

	ICoord2D toPixel(Vec2 world) const
	{
		// The radar shows +y up, like the world seen from above.
		return { x + (Int)(world.x / worldWidth * width), y + height - (Int)(world.y / worldHeight * height) };
	}
};

void drawBorder(const RadarArea &area, const World &w, const BorderEdge &edge)
{
	const PlayerId left = w.ownersAtRefresh()[edge.left];
	const PlayerId right = w.ownersAtRefresh()[edge.right];
	const ICoord2D a = area.toPixel(edge.a);
	const ICoord2D b = area.toPixel(edge.b);

	// Seen from above with +y up, a horizontal edge's left side is the pixel row above it, and a
	// vertical edge's left side is the pixel column to its left.
	const bool horizontal = a.y == b.y;
	const ICoord2D toLeft = horizontal ? ICoord2D{ 0, -1 } : ICoord2D{ -1, 0 };
	TheDisplay->drawLine(a.x + toLeft.x, a.y + toLeft.y, b.x + toLeft.x, b.y + toLeft.y, 1.0f, ownerColor(left, 230));
	TheDisplay->drawLine(a.x, a.y, b.x, b.y, 1.0f, ownerColor(right, 230));
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
	for (const BorderEdge &edge : w.map().borders())
		drawBorder(area, w, edge);
	for (int i = 0; i < w.sectorCount(); ++i)
		drawPoint(area, w, i);
}

} // namespace CoH::Sectors
