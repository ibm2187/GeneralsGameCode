// CoH Sectors: sectors drawn on the ground, in the style of Company of Heroes' territory lines. Each
// player's territory has one smooth outline that follows the ground: a crisp line in the owner's color
// with a soft band fading inward. The neutral ground's outline is a dim dashed line. Between sectors of
// one owner (or two neutral ones) there is only a faint line. A sector's edge flashes when it changes
// hands. Each point has a ring in its owner's color with the capture progress as an inner arc; a
// contested point's ring blinks, and a cut-off sector's ring is broken.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameClient/Display.h"
#include "GameClient/GameClient.h"
#include "GameClient/View.h"
#include "GameLogic/TerrainLogic.h"

#include "Kernel/Hooks.h"
#include "Kernel/Log.h"

#include "Features/Sectors/Engine/Config.h"
#include "Features/Sectors/Engine/Internal.h"
#include "Features/Sectors/Engine/SectorPointUpdate.h"
#include "Features/Sectors/Engine/World.h"
#include "Features/Sectors/UI/BorderGeometry.h"
#include "Features/Sectors/UI/Colors.h"

#include <chrono>
#include <cmath>

namespace CoH::Sectors
{

namespace
{

const Int RING_SEGMENTS = 40;
const Real CULL_MARGIN = 250.0f;	///< world units around the screen still drawn, for hills
const Int DASH_POINTS = 2;			///< points on, then off, in a dashed line
const Real FLASH_SECONDS = 3.0f;	///< how long a border flashes after a sector changes hands
const Real OCCLUSION_STEP = 25.0f;	///< world units between terrain samples looking for a hill in the way
const Real OCCLUSION_START = 20.0f;	///< ground this near a point never hides it (its own slope)
const Real OCCLUSION_SLACK = 4.0f;	///< the terrain must stand this far above the sight line to hide

const UnsignedByte BAND_ALPHA = 110;	///< the band's alpha at the crisp line; it fades to nothing inward

Color mix(Color from, Color to, Real amount)
{
	UnsignedByte r1, g1, b1, a1, r2, g2, b2, a2;
	GameGetColorComponents(from, &r1, &g1, &b1, &a1);
	GameGetColorComponents(to, &r2, &g2, &b2, &a2);
	const auto blend = [amount](UnsignedByte x, UnsignedByte y) { return (UnsignedByte)(x + (y - x) * amount); };
	return GameMakeColor(blend(r1, r2), blend(g1, g2), blend(b1, b2), blend(a1, a2));
}

// ---- What the camera sees -----------------------------------------------------------------------

struct Frame
{
	bool cull = false;
	Real minX = 0, minY = 0, maxX = 0, maxY = 0;	///< the ground on screen, with a margin

	bool occludes = false;	///< whether hills hide what is behind them: the camera was found
	Coord3D camera;
	Real highestGround = 0;
};

// The highest ground on the map, so a sight line above it can stop looking for hills.
Real highestGround(const World &w)
{
	static int buildNumber = -1;
	static Real highest = 0;
	if (buildNumber == w.buildNumber())
		return highest;

	buildNumber = w.buildNumber();
	highest = -1e9f;
	const SectorMap &map = w.map();
	for (int y = 0; y <= map.gridHeight(); ++y)
		for (int x = 0; x <= map.gridWidth(); ++x)
			highest = std::max(highest, TheTerrainLogic->getGroundHeight(x * map.cellSize(), y * map.cellSize()));
	highest += OCCLUSION_SLACK;
	return highest;
}

// Where the camera is: where the sight lines through two screen corners meet.
bool findCamera(Real z, Coord3D &camera)
{
	const ICoord2D pixels[2] = { { 0, 0 }, { TheTacticalView->getWidth(), TheTacticalView->getHeight() } };
	Coord3D low[2], high[2];
	for (int i = 0; i < 2; ++i)
	{
		if (TheTacticalView->screenToWorldAtZ(&pixels[i], &low[i], z) == PlaneClass::NO_INTERSECTION
			|| TheTacticalView->screenToWorldAtZ(&pixels[i], &high[i], z + 500.0f) == PlaneClass::NO_INTERSECTION)
			return false;
	}

	// The nearest points of the two lines p + s * u and q + t * v.
	const Coord3D p = low[0], q = low[1];
	const Coord3D u = { high[0].x - p.x, high[0].y - p.y, high[0].z - p.z };
	const Coord3D v = { high[1].x - q.x, high[1].y - q.y, high[1].z - q.z };
	const Coord3D r = { p.x - q.x, p.y - q.y, p.z - q.z };
	const auto dot = [](const Coord3D &a, const Coord3D &b) { return a.x * b.x + a.y * b.y + a.z * b.z; };
	const Real a = dot(u, u), b = dot(u, v), c = dot(v, v), d = dot(u, r), e = dot(v, r);
	const Real denominator = a * c - b * b;
	if (std::fabs(denominator) < 1e-6f)
		return false;
	const Real sAt = (b * e - c * d) / denominator;
	const Real tAt = (a * e - b * d) / denominator;
	camera = { (p.x + sAt * u.x + q.x + tAt * v.x) * 0.5f, (p.y + sAt * u.y + q.y + tAt * v.y) * 0.5f,
		(p.z + sAt * u.z + q.z + tAt * v.z) * 0.5f };
	return camera.z > z;
}

// Whether a hill stands between the camera and a point on the ground: walks the sight line toward
// the camera until it is above the highest ground.
bool isBehindHill(const Frame &frame, const Coord3D &point)
{
	if (!frame.occludes)
		return false;
	const Real dx = frame.camera.x - point.x;
	const Real dy = frame.camera.y - point.y;
	const Real dz = frame.camera.z - point.z;
	const Real length = std::sqrt(dx * dx + dy * dy + dz * dz);
	if (length <= OCCLUSION_START)
		return false;
	for (Real along = OCCLUSION_START; along < length; along += OCCLUSION_STEP)
	{
		const Real t = along / length;
		const Real z = point.z + dz * t;
		if (z > frame.highestGround)
			return false;
		if (TheTerrainLogic->getGroundHeight(point.x + dx * t, point.y + dy * t) > z + OCCLUSION_SLACK)
			return true;
	}
	return false;
}

// A straight line on the ground between two points, unless a hill hides either end.
void drawWorldLine(const Frame &frame, Vec2 a, Vec2 b, Real width, Color color)
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
	if (isBehindHill(frame, worldA) || isBehindHill(frame, worldB))
		return;
	TheDisplay->drawLine(screenA.x, screenA.y, screenB.x, screenB.y, width, color);
}

Frame seeFrame(const World &w)
{
	Frame frame;
	const Coord3D &target = TheTacticalView->getPosition();
	const Real z = TheTerrainLogic->getGroundHeight(target.x, target.y);
	frame.occludes = findCamera(z, frame.camera);
	frame.highestGround = highestGround(w);

	Coord3D corners[4];
	if (TheTacticalView->getScreenCornerWorldPointsAtZ(&corners[0], &corners[1], &corners[2], &corners[3], z)
		== PlaneClass::INSIDE_SEGMENT)
	{
		frame.cull = true;
		frame.minX = frame.maxX = corners[0].x;
		frame.minY = frame.maxY = corners[0].y;
		for (const Coord3D &corner : corners)
		{
			frame.minX = std::min(frame.minX, corner.x);
			frame.minY = std::min(frame.minY, corner.y);
			frame.maxX = std::max(frame.maxX, corner.x);
			frame.maxY = std::max(frame.maxY, corner.y);
		}
		frame.minX -= CULL_MARGIN;
		frame.minY -= CULL_MARGIN;
		frame.maxX += CULL_MARGIN;
		frame.maxY += CULL_MARGIN;
	}

	return frame;
}

bool isOnScreen(const Frame &frame, const GroundStrip &strip)
{
	return !frame.cull
		|| (strip.maxX >= frame.minX && strip.minX <= frame.maxX && strip.maxY >= frame.minY && strip.minY <= frame.maxY);
}

// ---- Sectors changing hands ---------------------------------------------------------------------

class Flashes
{
public:
	// Notes the owners as of this frame; a sector whose owner changed starts flashing.
	void update(const World &w)
	{
		const std::vector<PlayerId> &owners = w.ownersAtRefresh();
		const UnsignedInt now = TheGameClient->getFrame();
		if (m_buildNumber != w.buildNumber() || m_owners.size() != owners.size())
		{
			m_buildNumber = w.buildNumber();
			m_owners = owners;
			m_changedAt.assign(owners.size(), 0);
			m_flashing.assign(owners.size(), false);
			return;
		}
		for (size_t i = 0; i < owners.size(); ++i)
		{
			if (owners[i] != m_owners[i])
			{
				m_owners[i] = owners[i];
				m_changedAt[i] = now;
				m_flashing[i] = true;
			}
		}
		m_now = now;
	}

	// 1 just after a change, pulsing down to 0.
	Real of(int sector) const
	{
		if (!m_flashing[sector])
			return 0.0f;
		const Real seconds = (m_now - m_changedAt[sector]) / (Real)LOGICFRAMES_PER_SECOND;
		if (seconds >= FLASH_SECONDS)
			return 0.0f;
		const Real fade = 1.0f - seconds / FLASH_SECONDS;
		return fade * (0.6f + 0.4f * std::cos(seconds * 2.0f * PI * 1.5f));
	}

private:
	int m_buildNumber = -1;
	UnsignedInt m_now = 0;
	std::vector<PlayerId> m_owners;
	std::vector<UnsignedInt> m_changedAt;
	std::vector<bool> m_flashing;
};

Flashes &flashes()
{
	static Flashes theFlashes;
	return theFlashes;
}

// ---- Borders ------------------------------------------------------------------------------------

struct Style
{
	bool visible = false;
	Color color = 0;
	Real width = 1.0f;
	bool dashed = false;
};

Style styleOf(const GroundStrip &strip, const std::vector<PlayerId> &owners)
{
	Style style;
	switch (strip.kind)
	{
	case GroundStrip::Kind::Center:
	{
		const PlayerId owner = owners[strip.side];
		style.visible = owner == owners[strip.other];
		style.dashed = owner == NoPlayer;
		style.color = ownerColor(owner, owner == NoPlayer ? 60 : 70);
		style.width = 1.5f;
		break;
	}

	case GroundStrip::Kind::Flash:
	{
		const Real flash = flashes().of(strip.side);
		style.visible = flash > 0.0f;
		if (style.visible)
		{
			style.color = mix(ownerColor(owners[strip.side], 255), GameMakeColor(255, 255, 255, 255), 0.6f);
			style.color = withAlpha(style.color, (UnsignedByte)(255.0f * std::min(1.0f, flash)));
			style.width = 2.0f + 3.0f * flash;
		}
		break;
	}

	case GroundStrip::Kind::Edge:
		style.visible = true;
		style.dashed = strip.owner == NoPlayer;
		style.color = ownerColor(strip.owner, strip.owner == NoPlayer ? 130 : 235);
		style.width = strip.owner == NoPlayer ? 1.5f : 2.5f;
		break;

	case GroundStrip::Kind::Band:
		style.visible = true;
		style.color = ownerColor(strip.owner, BAND_ALPHA);
		break;
	}
	return style;
}

// Projects points to the screen. A point behind a hill counts as not drawable (invalid).
void project(const Frame &frame, const std::vector<Coord3D> &points, size_t offset, std::vector<ICoord2D> &screen,
	std::vector<View::WorldToScreenReturn> &where)
{
	for (size_t i = 0; i < points.size(); ++i)
	{
		View::WorldToScreenReturn &result = where[offset + i];
		result = TheTacticalView->worldToScreenTriReturn(&points[i], &screen[offset + i]);
		if (result == View::WTS_INSIDE_FRUSTUM && isBehindHill(frame, points[i]))
			result = View::WTS_INVALID;
	}
}

void drawStrip(const Frame &frame, const GroundStrip &strip, const Style &style, std::vector<ICoord2D> &screen,
	std::vector<View::WorldToScreenReturn> &where)
{
	const size_t n = strip.points.size();
	screen.resize(n);
	where.resize(n);
	project(frame, strip.points, 0, screen, where);

	for (size_t i = 0; i + 1 < n; ++i)
	{
		if (style.dashed && ((strip.firstIndex + (Int)i) / DASH_POINTS) % 2 == 1)
			continue;
		if (where[i] == View::WTS_INVALID || where[i + 1] == View::WTS_INVALID)
			continue;
		if (where[i] != View::WTS_INSIDE_FRUSTUM && where[i + 1] != View::WTS_INSIDE_FRUSTUM)
			continue;
		TheDisplay->drawLine(screen[i].x, screen[i].y, screen[i + 1].x, screen[i + 1].y, style.width, style.color);
	}
}

// The band as one gradient: each piece between the crisp line and the band's inner edge is filled,
// shaded from the owner's color at the line to nothing inside.
void drawBand(const Frame &frame, const GroundStrip &strip, const Style &style, std::vector<ICoord2D> &screen,
	std::vector<View::WorldToScreenReturn> &where)
{
	const size_t n = strip.points.size();
	screen.resize(2 * n);
	where.resize(2 * n);
	project(frame, strip.points, 0, screen, where);
	project(frame, strip.inner, n, screen, where);

	const Color clear = withAlpha(style.color, 0);
	for (size_t i = 0; i + 1 < n; ++i)
	{
		const size_t corner[4] = { i, i + 1, n + i, n + i + 1 };	// the line's two points, then the inner edge's
		bool valid = true;
		bool inside = false;
		float corners[8];
		for (int k = 0; k < 4; ++k)
		{
			valid = valid && where[corner[k]] != View::WTS_INVALID;
			inside = inside || where[corner[k]] == View::WTS_INSIDE_FRUSTUM;
			corners[2 * k] = (float)screen[corner[k]].x;
			corners[2 * k + 1] = (float)screen[corner[k]].y;
		}
		if (valid && inside)
			CoH::Hooks::drawGradientQuad(corners, style.color, clear);
	}
}

void drawStrips(const std::vector<GroundStrip> &strips, GroundStrip::Kind kind, const std::vector<PlayerId> &owners,
	const Frame &frame)
{
	static std::vector<ICoord2D> screen;
	static std::vector<View::WorldToScreenReturn> where;
	for (const GroundStrip &strip : strips)
	{
		if (strip.kind != kind || !isOnScreen(frame, strip))
			continue;
		const Style style = styleOf(strip, owners);
		if (!style.visible)
			continue;
		if (kind == GroundStrip::Kind::Band)
			drawBand(frame, strip, style, screen, where);
		else
			drawStrip(frame, strip, style, screen, where);
	}
}

void drawBorders(const World &w, const Frame &frame)
{
	const std::vector<PlayerId> &owners = w.ownersAtRefresh();
	const SectorGeometry &sectors = sectorGeometry(w);
	const TerritoryGeometry &territory = territoryGeometry(w);

	// The soft bands under everything, then the faint lines inside territories, the crisp edges, and
	// the flashes on top.
	drawStrips(territory.strips, GroundStrip::Kind::Band, owners, frame);
	drawStrips(sectors.strips, GroundStrip::Kind::Center, owners, frame);
	drawStrips(territory.strips, GroundStrip::Kind::Edge, owners, frame);
	drawStrips(sectors.strips, GroundStrip::Kind::Flash, owners, frame);
}

// ---- Points -------------------------------------------------------------------------------------

Vec2 onCircle(Vec2 center, Real radius, Real turn)
{
	const Real angle = turn * 2.0f * PI + PI / 2.0f;	// from the top, clockwise on screen
	return { center.x + std::cos(angle) * radius, center.y + std::sin(angle) * radius };
}

// From the top of the circle, 'fraction' of the way round.
void drawArc(const Frame &frame, Vec2 center, Real radius, Real fraction, Real width, Color color, bool broken)
{
	const Int segments = (Int)std::ceil(RING_SEGMENTS * fraction);
	for (Int i = 0; i < segments; ++i)
	{
		if (broken && i % 2 == 1)
			continue;
		const Real from = (Real)i / RING_SEGMENTS;
		const Real to = std::min(fraction, (Real)(i + 1) / RING_SEGMENTS);
		drawWorldLine(frame, onCircle(center, radius, -from), onCircle(center, radius, -to), width, color);
	}
}

void drawPoint(const World &w, const Frame &frame, int index)
{
	const SectorPointUpdate *point = w.point(index);
	if (point == nullptr)
		return;

	const PointState &state = point->state();
	const Vec2 center = w.sector(index).position;
	const Real radius = config().captureRadius;
	if (frame.cull && (center.x + radius < frame.minX || center.x - radius > frame.maxX
		|| center.y + radius < frame.minY || center.y - radius > frame.maxY))
		return;

	const bool contested = w.occupation(index).kind == Occupation::Kind::Contested;
	const bool cutOff = w.isCutOff(index);

	Color ring = ownerColor(state.owner, 220);
	if (contested && blinkOn())
		ring = GameMakeColor(255, 255, 255, 230);
	drawArc(frame, center, radius, 1.0f, 2.0f, ring, cutOff);

	// The capture progress: the owner's hold while owned, else whoever is taking it.
	const bool full = state.owner != NoPlayer && state.progress >= 1.0f;
	if (!full && state.progress > 0.0f)
		drawArc(frame, center, radius * 0.85f, state.progress, 4.0f, ownerColor(state.progressSide, 255), false);

	if (w.sector(index).kind == SectorKind::HighValue)
		drawArc(frame, center, radius * 0.7f, 1.0f, 1.0f, ownerColor(state.owner, 160), false);
}

} // namespace

void drawGround()
{
	const World &w = world();
	if (!w.hasSectors() || TheTacticalView == nullptr || TheDisplay == nullptr || TheTerrainLogic == nullptr
		|| TheGameClient == nullptr)
		return;

	const auto started = std::chrono::steady_clock::now();
	flashes().update(w);
	const Frame frame = seeFrame(w);
	drawBorders(w, frame);
	for (int i = 0; i < w.sectorCount(); ++i)
		drawPoint(w, frame, i);

	// With -cohLog, what the drawing costs, averaged every 30 seconds (this side only; the GPU draws
	// the batch later).
	if (CoH::isLogging())
	{
		static double totalMs = 0.0, worstMs = 0.0;
		static int frames = 0;
		static auto since = started;
		const double ms = std::chrono::duration<double, std::milli>(std::chrono::steady_clock::now() - started).count();
		totalMs += ms;
		worstMs = std::max(worstMs, ms);
		++frames;
		if (std::chrono::steady_clock::now() - since >= std::chrono::seconds(30))
		{
			CoH::log("Sectors: ground drawing %.3f ms a frame on average, %.3f ms at worst, over %d frames",
				totalMs / frames, worstMs, frames);
			totalMs = worstMs = 0.0;
			frames = 0;
			since = std::chrono::steady_clock::now();
		}
	}
}

} // namespace CoH::Sectors
