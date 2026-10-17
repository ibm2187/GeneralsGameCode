// CoH hook hub: see Kernel/Hooks.h. Each hook does the Kernel's part of the engine event, then
// raises the event for the subscribed features that are on.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/Object.h"

#include "Kernel/Domain/GameFlags.h"
#include "Kernel/Engine/Internal.h"
#include "Kernel/Hooks.h"

namespace CoH::Hooks
{

// ---- Events ------------------------------------------------------------------------------------

Event<bool> &gameStartBeforeMap() { static Event<bool> event; return event; }
Event<bool> &gameStartAfterMap() { static Event<bool> event; return event; }
Event<> &logicFrame() { static Event<> event; return event; }
Event<Object *> &objectCreated() { static Event<Object *> event; return event; }
Event<Object *> &objectDestroyed() { static Event<Object *> event; return event; }
Event<> &gameEnd() { static Event<> event; return event; }
Veto<const ThingTemplate *, const Coord3D *, const Player *> &buildLocation()
{
	static Veto<const ThingTemplate *, const Coord3D *, const Player *> veto;
	return veto;
}

Event<> &inGameUIPostDraw() { static Event<> event; return event; }
Event<int, int, int, int> &radarDraw() { static Event<int, int, int, int> event; return event; }
Event<const Player *, UnicodeString &> &moneyTooltip() { static Event<const Player *, UnicodeString &> event; return event; }

// ---- Engine ------------------------------------------------------------------------------------

void onEngineInit()
{
	Kernel::init();
	QuickSkirmish::prepareIfAsked();
}

void onShellShown()
{
	QuickSkirmish::startIfAsked();
}

void onModuleFactoryInit(ModuleFactory &factory)
{
	Registration::addModulesTo(factory);
}

INIBlockParse findIniBlock(const char *token)
{
	return Registration::findIniBlock(token);
}

// ---- Game --------------------------------------------------------------------------------------

void onGameStartBeforeMap(bool loadingSaveGame)
{
	FeatureRegistry &registry = Kernel::registry();
	registry.activate(registry.known(GameFlags::forNewGame(loadingSaveGame)));
	GameFlags::clearSaved();
	DEBUG_LOG(("CoH: game starts with flags '%s'", registry.active().toList().c_str()));

	Packs::loadActive(registry);
	gameStartBeforeMap().raise(loadingSaveGame);
}

void onGameStartAfterMap(bool loadingSaveGame)
{
	gameStartAfterMap().raise(loadingSaveGame);
}

void onLogicFrame()
{
	logicFrame().raise();
	QuickSkirmish::quitIfDue();
}

void onObjectCreated(Object *object)
{
	objectCreated().raise(object);
}

void onObjectDestroyed(Object *object)
{
	objectDestroyed().raise(object);
}

// The engine resets after every game and once at startup. The packs' overrides are dropped by the
// engine's own reset right after this, like map.ini's.
void onGameEnd()
{
	gameEnd().raise();

	FeatureRegistry &registry = Kernel::registry();
	registry.resetAll();
	registry.deactivateAll();
	GameFlags::clearSaved();
}

bool isBuildLocationAllowed(const ThingTemplate *build, const Coord3D *position, const Player *player,
	const Object *builder)
{
	if (player == nullptr && builder != nullptr)
		player = builder->getControllingPlayer();
	return buildLocation().allows(build, position, player);
}

// ---- Client ------------------------------------------------------------------------------------

void onInGameUIPostDraw()
{
	inGameUIPostDraw().raise();
}

void onRadarDraw(int x, int y, int width, int height)
{
	radarDraw().raise(x, y, width, height);
}

void onMoneyTooltip(const Player *player, UnicodeString &description)
{
	moneyTooltip().raise(player, description);
}

// ---- Game options ------------------------------------------------------------------------------

AsciiString gameOptions(const GameInfo *game)
{
	return GameFlags::optionsFor(game);
}

bool isGameOptionKey(const AsciiString &key)
{
	return GameOptionKey == key.str();
}

void onGameOptionsParsed(GameInfo *game, const AsciiString &options)
{
	GameFlags::onOptionsParsed(game, options);
}

} // namespace CoH::Hooks
