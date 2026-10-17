// CoH hook hub: the one place original code calls into CoH, and where features subscribe.
//
// Original code calls the on...() functions below. Each call site is marked "// CoH hook" and is
// listed in the hooks table in CoH/README.md. Features never edit original code for an event the
// hub already has; they subscribe to its Event in their init. A feature that needs a new point adds
// one general point here, named after the engine event, and lists its call site in the table.

#pragma once

#include "Kernel/Domain/Event.h"

class AsciiString;
class CommandButton;
class GameInfo;
class GameWindow;
class INI;
class ModuleFactory;
class Object;
class Player;
class Snapshot;
class SpecialPowerTemplate;
class ThingTemplate;
class UnicodeString;
struct Coord3D;
class DamageInfo;
typedef void (*INIBlockParse)( INI *ini );

namespace CoH::Hooks
{

// ---- Called from original code -----------------------------------------------------------------

bool allowsSeveralGames();	///< CommandLine startup: a quick test skirmish may run beside other games
void onEngineInit();		///< GameEngine::init, once GameData and the command line are read
void onShellShown();		///< GameClient::update, the intro is over and the shell is up, as -loadsave starts
void onModuleFactoryInit(ModuleFactory &factory);	///< ModuleFactory::init, after the original modules
INIBlockParse findIniBlock(const char *token);	///< INI block table lookup, for a block it doesn't know
Snapshot *saveBlock();		///< GameState::init: the save block, saved before the map

void onGameStartBeforeMap(bool loadingSaveGame);	///< GameLogic::startNewGame, before map.ini loads
void onGameStartAfterMap(bool loadingSaveGame);		///< GameLogic::startNewGame, map and players set up
void onLogicFrame();		///< GameLogic::update, after objects update, before destroyed ones are removed
void onObjectCreated(Object *object);		///< ThingFactory::newObject, once the object is set up
void onObjectDestroyed(Object *object);		///< GameLogic::destroyObject, once the object is marked destroyed
void onGameEnd();			///< GameLogic::reset, once every object is gone
bool isBuildLocationAllowed(const ThingTemplate *build, const Coord3D *position, const Player *player,
	const Object *builder);	///< BuildAssistant::isLocationLegalToBuild; the player is the builder's when null
void onKillScored(Object *killer, const Object *victim);	///< Object::scoreTheKill: an enemy's kill is credited
void onDamageDealt(Object *victim, const DamageInfo *damage);	///< ActiveBody::attemptDamage: health was taken
bool isSpecialPowerAllowed(const Object *source, const SpecialPowerTemplate *power);	///< SpecialPowerStore::canUseSpecialPower, ControlBar
void onSpecialPowerTriggered(Object *source, const SpecialPowerTemplate *power, const Coord3D *target);	///< SpecialPowerModule::triggerSpecialPower
bool isProductionAllowed(const Player *player, const ThingTemplate *build);	///< Player::canBuildMoreOfType
void onIncome(const Player *player, unsigned int &amount);	///< Money::deposit: income (not starting cash, sales or refunds)

void onInGameUIPostDraw();	///< InGameUI::postDraw: over the 3D view, under the windows
void onRadarDraw(int x, int y, int width, int height);	///< W3DRadar::draw: over the map and shroud, under the icons
void onMoneyTooltip(const Player *player, UnicodeString &description);	///< ControlBar: the money tooltip's text is made
void onCommandTooltip(const CommandButton *button, UnicodeString &description);	///< ControlBar: a button's tooltip text is made
bool onMoneyDisplay(const Player *player, GameWindow *moneyWindow);	///< InGameUI: true when a feature writes the money text
const UnicodeString *findText(const char *label);	///< GameTextManager::fetch: text a pack adds, for a label the game lacks

AsciiString gameOptions(const GameInfo *game);	///< appended to the game options string; empty with no flags
bool isGameOptionKey(const AsciiString &key);	///< tells the options parser the CoH key is known
void onGameOptionsParsed(GameInfo *game, const AsciiString &options);	///< a game options string was read

// ---- Subscribed to by features, in their init. A subscriber runs only while its feature is on. --

Event<bool> &gameStartBeforeMap();	///< loading a save or not. The feature's INI pack is loaded; map.ini and the map are not.
Event<bool> &gameStartAfterMap();	///< loading a save or not. Map, players and starting units are in; a save's objects come after.
Event<> &logicFrame();
Event<Object *> &objectCreated();
Event<Object *> &objectDestroyed();
Event<> &gameEnd();					///< every object is gone; the feature's reset runs next

// The structure 'build' may go at 'position' for 'player' (which may be null). Saying no makes the
// spot illegal for the player's placement cursor and for the AI.
Veto<const ThingTemplate *, const Coord3D *, const Player *> &buildLocation();

Event<Object *, const Object *> &killScored();		///< the killer, the enemy it killed; credited like skill points
Event<Object *, const DamageInfo *> &damageDealt();	///< the victim and the damage it took (out.m_actualDamageClipped)

// May 'source' use 'power' now? Asked when a power is used (not when scripts force it) and for its
// button, which a no greys out. Powers no feature knows are always allowed.
Veto<const Object *, const SpecialPowerTemplate *> &specialPowerUse();
Event<Object *, const SpecialPowerTemplate *, const Coord3D *> &specialPowerTriggered();	///< fired; its recharge has started. The target may be null.

// May 'player' make one more 'build' (a unit or a structure)? Asked for the build buttons, which a no
// greys, when an order is queued, and by the AI looking for a factory, so a no makes it pick another.
Veto<const Player *, const ThingTemplate *> &production();

// Income arrives for the player: gathered supplies, sector income, oil derricks, hackers. A subscriber
// may lower the amount; what is left is deposited and counted as income.
Event<const Player *, unsigned int &> &income();

// ---- Client: drawing and text. These don't run in -headless games. ------------------------------

Event<> &inGameUIPostDraw();			///< draw over the 3D view with TheDisplay, in screen pixels
Event<int, int, int, int> &radarDraw();	///< the radar's drawn map area on screen: x, y, width, height
Event<const Player *, UnicodeString &> &moneyTooltip();	///< the viewed player; the description to add to
Event<const CommandButton *, UnicodeString &> &commandTooltip();	///< a command button's tooltip; the description to add to
Event<const Player *, UnicodeString &> &moneyDisplay();	///< the viewed player; the money text ("$ 1200") to add to

// ---- Client: drawing the display offers but TheDisplay doesn't. Original code provides it. ---------

// A filled four-cornered shape in screen pixels, shaded from color1 at corners 0 and 1 to color2 at
// corners 2 and 3; corners 1 and 2 are on its diagonal. 'corners' is x0, y0, ... x3, y3.
using GradientQuadDraw = void (*)(const float corners[8], unsigned int color1, unsigned int color2);
void provideGradientQuad(GradientQuadDraw draw);	///< W3DDisplay::init
bool drawGradientQuad(const float corners[8], unsigned int color1, unsigned int color2);	///< false while no display provides it

} // namespace CoH::Hooks
