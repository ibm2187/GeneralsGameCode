// CoH hook hub: see Kernel/Hooks.h. Each hook does the Kernel's part of the engine event, then
// raises the event for the subscribed features that are on.

#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "Common/Money.h"
#include "Common/Player.h"
#include "GameClient/GadgetStaticText.h"
#include "GameClient/GameText.h"
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

Event<Object *, const Object *> &killScored() { static Event<Object *, const Object *> event; return event; }
Event<Object *, const DamageInfo *> &damageDealt() { static Event<Object *, const DamageInfo *> event; return event; }
Veto<const Object *, const SpecialPowerTemplate *> &specialPowerUse()
{
	static Veto<const Object *, const SpecialPowerTemplate *> veto;
	return veto;
}
Event<Object *, const SpecialPowerTemplate *, const Coord3D *> &specialPowerTriggered()
{
	static Event<Object *, const SpecialPowerTemplate *, const Coord3D *> event;
	return event;
}

Event<> &inGameUIPostDraw() { static Event<> event; return event; }
Event<int, int, int, int> &radarDraw() { static Event<int, int, int, int> event; return event; }
Event<const Player *, UnicodeString &> &moneyTooltip() { static Event<const Player *, UnicodeString &> event; return event; }
Event<const CommandButton *, UnicodeString &> &commandTooltip() { static Event<const CommandButton *, UnicodeString &> event; return event; }
Event<const Player *, UnicodeString &> &moneyDisplay() { static Event<const Player *, UnicodeString &> event; return event; }

// ---- Engine ------------------------------------------------------------------------------------

// Quick test skirmishes (-cohSkirmish) may run side by side, as replay playback may. Each game then
// gets its own replay (the engine's _InstanceNN names) and its own CoH log.
bool allowsSeveralGames()
{
	return QuickSkirmish::isAsked();
}

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
	PackText::clear();
}

bool isBuildLocationAllowed(const ThingTemplate *build, const Coord3D *position, const Player *player,
	const Object *builder)
{
	if (player == nullptr && builder != nullptr)
		player = builder->getControllingPlayer();
	return buildLocation().allows(build, position, player);
}

void onKillScored(Object *killer, const Object *victim)
{
	killScored().raise(killer, victim);
}

void onDamageDealt(Object *victim, const DamageInfo *damage)
{
	damageDealt().raise(victim, damage);
}

bool isSpecialPowerAllowed(const Object *source, const SpecialPowerTemplate *power)
{
	return specialPowerUse().allows(source, power);
}

void onSpecialPowerTriggered(Object *source, const SpecialPowerTemplate *power, const Coord3D *target)
{
	specialPowerTriggered().raise(source, power, target);
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

void onCommandTooltip(const CommandButton *button, UnicodeString &description)
{
	commandTooltip().raise(button, description);
}

// With a feature adding to it, the money text is made here every frame: what features add (such as
// command points) changes without the money changing, which is all the original code watches.
bool onMoneyDisplay(const Player *player, GameWindow *moneyWindow)
{
	if (!moneyDisplay().hasActiveSubscriber() || player == nullptr || moneyWindow == nullptr)
		return false;

	UnicodeString text;
	text.format(TheGameText->fetch("GUI:ControlBarMoneyDisplay"), player->getMoney()->countMoney());
	moneyDisplay().raise(player, text);

	static UnicodeString shown;
	if (text != shown || GadgetStaticTextGetText(moneyWindow) != text)
	{
		GadgetStaticTextSetText(moneyWindow, text);
		shown = text;
	}
	return true;
}

const UnicodeString *findText(const char *label)
{
	return PackText::find(label);
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
