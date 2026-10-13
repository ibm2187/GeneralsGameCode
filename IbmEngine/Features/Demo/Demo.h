// IBM Engine: the Demo flag, a ready-made test setup at game start for the features that are on
// (Squads, Economy). See README.md.

#pragma once

namespace IbmEngine
{

// GameLogic::startNewGame, once the players' starting bases are placed (not when loading a save).
void setUpDemo();

// GameEngine::init, once the map list is built: with "-demo [map]" on the command line, starts a
// skirmish on that map (default Maps\IbmDemo\IbmDemo.map) against one AI, skipping the menus, and
// returns TRUE. DemoLaunch.cpp.
Bool startDemoGame();

} // namespace IbmEngine
