// CoH GeneralsArsenal: super weapon targets and air cover zones drawn for the players.
//
// A super weapon target is shown to every player, through the shroud: a ring on the ground that
// blinks faster as the end nears, the weapon's name and a countdown above it, and a blinking mark on
// the minimap. So everyone can see it coming and scatter. An air cover zone is shown to its owner and
// allies only, as a quiet ring.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/GameUtility.h"
#include "Common/GlobalData.h"
#include "Common/Player.h"
#include "Common/PlayerList.h"
#include "GameClient/Display.h"
#include "GameClient/DisplayString.h"
#include "GameClient/DisplayStringManager.h"
#include "GameClient/GameClient.h"
#include "GameClient/GameFont.h"
#include "GameClient/GameText.h"
#include "GameClient/GlobalLanguage.h"
#include "GameClient/View.h"
#include "GameLogic/GameLogic.h"
#include "GameLogic/Object.h"
#include "GameLogic/TerrainLogic.h"

#include "Features/GeneralsArsenal/Domain/Warning.h"
#include "Features/GeneralsArsenal/Engine/AirCoverZoneUpdate.h"
#include "Features/GeneralsArsenal/Engine/Internal.h"
#include "Features/GeneralsArsenal/Engine/StrikeCountdownUpdate.h"

#include <cmath>

namespace CoH::GeneralsArsenal
{

namespace
{

const Int RING_SEGMENTS = 48;
const Real LIFT = 2.0f;

const Color WARNING_COLOR = GameMakeColor(255, 40, 30, 230);
const Color WARNING_DIM = GameMakeColor(255, 40, 30, 110);
const Color COVER_COLOR = GameMakeColor(120, 200, 255, 120);

Coord3D onGround(Real x, Real y)
{
	Coord3D world = { x, y, 0.0f };
	world.z = TheTerrainLogic->getGroundHeight(x, y) + LIFT;
	return world;
}

void drawWorldLine(const Coord3D &a, const Coord3D &b, Real width, Color color)
{
	ICoord2D screenA, screenB;
	const View::WorldToScreenReturn ra = TheTacticalView->worldToScreenTriReturn(&a, &screenA);
	const View::WorldToScreenReturn rb = TheTacticalView->worldToScreenTriReturn(&b, &screenB);
	if (ra == View::WTS_INVALID || rb == View::WTS_INVALID)
		return;
	if (ra != View::WTS_INSIDE_FRUSTUM && rb != View::WTS_INSIDE_FRUSTUM)
		return;
	TheDisplay->drawLine(screenA.x, screenA.y, screenB.x, screenB.y, width, color);
}

void drawRing(const Coord3D &center, Real radius, Real width, Color color)
{
	for (Int i = 0; i < RING_SEGMENTS; ++i)
	{
		const Real from = 2.0f * PI * i / RING_SEGMENTS;
		const Real to = 2.0f * PI * (i + 1) / RING_SEGMENTS;
		drawWorldLine(onGround(center.x + cosf(from) * radius, center.y + sinf(from) * radius),
			onGround(center.x + cosf(to) * radius, center.y + sinf(to) * radius), width, color);
	}
}

// Faster as the end nears: every 16 frames at the start, every 4 at the end.
bool blinkOn(Real progress)
{
	const UnsignedInt period = progress > 0.75f ? 4 : (progress > 0.4f ? 8 : 16);
	return (TheGameClient->getFrame() / period) % 2 == 0;
}

DisplayString *&countdownTextStorage()
{
	static DisplayString *text = nullptr;
	return text;
}

DisplayString *countdownText()
{
	DisplayString *&text = countdownTextStorage();
	if (text == nullptr && TheDisplayStringManager != nullptr)
	{
		text = TheDisplayStringManager->newDisplayString();
		text->setFont(TheFontLibrary->getFont("Arial", TheGlobalLanguageData->adjustFontSize(14), TRUE));
	}
	return text;
}

// Button names mark their hotkey with '&' ("&Nuclear Missile"); the label over the target is plain text.
UnicodeString withoutHotkeyMark(const UnicodeString &text)
{
	UnicodeString plain;
	for (const WideChar *c = text.str(); *c; ++c)
	{
		if (*c != L'&')
			plain.concat(*c);
	}
	return plain;
}

void drawCountdown(const Object *marker, const StrikeCountdownUpdate &countdown)
{
	DisplayString *text = countdownText();
	if (text == nullptr)
		return;
	const Coord3D above = onGround(marker->getPosition()->x, marker->getPosition()->y);
	ICoord2D screen;
	if (TheTacticalView->worldToScreenTriReturn(&above, &screen) != View::WTS_INSIDE_FRUSTUM)
		return;

	const WarningTimer timer = countdown.timer();
	const std::string clock = formatCountdown(timer.secondsLeft(TheGameLogic->getFrame(), LOGICFRAMES_PER_SECOND));
	const UnicodeString name = withoutHotkeyMark(countdown.getLabel().isNotEmpty() ? TheGameText->fetch(countdown.getLabel()) : UnicodeString(L""));
	UnicodeString clockText;
	clockText.translate(AsciiString(clock.c_str()));
	UnicodeString line;
	line.format(L"%ls  %ls", name.str(), clockText.str());

	text->setText(line);
	text->draw(screen.x - text->getWidth() / 2, screen.y - 40, WARNING_COLOR, GameMakeColor(0, 0, 0, 255));
}

bool isShownTo(const Object *object, const Player *viewer, Bool everyone)
{
	if (everyone)
		return true;
	const Player *owner = object->getControllingPlayer();
	return viewer == nullptr || owner == viewer || (owner && viewer->getRelationship(owner->getDefaultTeam()) == ALLIES);
}

const Player *viewer()
{
	return rts::getObservedOrLocalPlayer_Safe();
}

} // namespace

void releaseMarkerText()
{
	DisplayString *&text = countdownTextStorage();
	if (text != nullptr && TheDisplayStringManager != nullptr)
		TheDisplayStringManager->freeDisplayString(text);
	text = nullptr;
}

void drawMarkersOnGround()
{
	if (TheTacticalView == nullptr || TheDisplay == nullptr || TheTerrainLogic == nullptr)
		return;

	for (Object *marker : strikeMarkers())
	{
		const StrikeCountdownUpdate *countdown = StrikeCountdownUpdate::find(marker);
		if (countdown == nullptr || countdown->hasFired() || !isShownTo(marker, viewer(), countdown->warnsEveryone()))
			continue;
		const Real progress = countdown->timer().progress(TheGameLogic->getFrame());
		drawRing(*marker->getPosition(), countdown->getRadius(), 3.0f, blinkOn(progress) ? WARNING_COLOR : WARNING_DIM);
		drawRing(*marker->getPosition(), countdown->getRadius() * (1.0f - progress), 1.5f, WARNING_COLOR);
		drawCountdown(marker, *countdown);
	}

	for (Object *zone : airCoverZones())
	{
		const AirCoverZoneUpdate *cover = AirCoverZoneUpdate::find(zone);
		if (cover && cover->isPatrolling() && isShownTo(zone, viewer(), FALSE))
			drawRing(*zone->getPosition(), cover->getRadius(), 1.5f, COVER_COLOR);
	}
}

void drawMarkersOnRadar(int x, int y, int width, int height)
{
	if (TheDisplay == nullptr || TheTerrainLogic == nullptr || width <= 0 || height <= 0)
		return;
	Region3D extent;
	TheTerrainLogic->getExtent(&extent);
	if (extent.hi.x <= 0.0f || extent.hi.y <= 0.0f)
		return;

	for (Object *marker : strikeMarkers())
	{
		const StrikeCountdownUpdate *countdown = StrikeCountdownUpdate::find(marker);
		if (countdown == nullptr || countdown->hasFired() || !isShownTo(marker, viewer(), countdown->warnsEveryone()))
			continue;
		if (!blinkOn(countdown->timer().progress(TheGameLogic->getFrame())))
			continue;
		// The radar shows +y up, like the world seen from above.
		const Int px = x + (Int)(marker->getPosition()->x / extent.hi.x * width);
		const Int py = y + height - (Int)(marker->getPosition()->y / extent.hi.y * height);
		const Int size = std::max(6, (Int)(countdown->getRadius() * 2.0f / extent.hi.x * width));
		TheDisplay->drawOpenRect(px - size / 2, py - size / 2, size, size, 2.0f, WARNING_COLOR);
	}
}

} // namespace CoH::GeneralsArsenal
