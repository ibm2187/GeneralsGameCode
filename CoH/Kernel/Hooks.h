// CoH hook hub: the one place original code calls into CoH, and where features subscribe.
//
// Original code calls the on...() functions below. Each call site is marked "// CoH hook" and is
// listed in the hooks table in CoH/README.md. Features never edit original code for an event the
// hub already has; they subscribe to its Event in their init. A feature that needs a new point adds
// one general point here, named after the engine event, and lists its call site in the table.

#pragma once

#include "Kernel/Domain/Event.h"

class AsciiString;
class GameInfo;
class INI;
class ModuleFactory;
class Object;
class Snapshot;
typedef void (*INIBlockParse)( INI *ini );

namespace CoH::Hooks
{

// ---- Called from original code -----------------------------------------------------------------

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

} // namespace CoH::Hooks
