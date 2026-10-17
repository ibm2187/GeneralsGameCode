# CoH

Company of Heroes ideas for Zero Hour, kept apart from the original code. [PLAN.md](PLAN.md) has the direction, the conventions and the roadmap; [Tasks](Tasks) has one file per feature.

Everything lives in this folder:
- `Kernel/` is shared by every feature: the feature registry, the flags, the INI packs and the hook hub.
- `Features/<Name>/` holds one slice per feature.

The original code only calls the hook hub, from the places listed in [Hooks in original code](#hooks-in-original-code).

## Features

| Feature | Flag | Task | Status |
|---|---|---|---|
| (none yet) | | | |

Every feature has a flag. With every flag off the game plays exactly like `main`, replays included.

## Flags

Local games use the local flags. They come from one of two places:

- `Features.ini` in the user data folder (`Documents\Command and Conquer Generals Zero Hour Data`), one feature per line:
  ```ini
  ; CoH features for my own games
  Sectors = yes
  Accuracy = no
  ```
- `-coh Sectors,Accuracy` on the command line. When it is given, it replaces `Features.ini` for that run, and `-coh none` turns every feature off.

Names are matched without case. A name with no feature in the build is ignored. A stale `Features.ini` from the old IbmEngine builds lists names such as `Accuracy` and `Squads`, so those features turn on as soon as they exist. Edit the file if that isn't wanted.

Each game decides which flags it plays with:

| Game | Flags |
|---|---|
| Shell map (the menu background) | none |
| Skirmish, campaign, or a network game you host | the local flags |
| A network game someone else hosts | the host's flags, sent in the game options (`CoH=Sectors,Accuracy;`) |
| A replay | the flags it was recorded with; the game options string is stored in the replay header |
| A save game | the flags it was saved with (the `CHUNK_CoH` save block) |

With no flags on, the game options string is exactly the original, so replays recorded with no flags stay compatible with `main`. Every player of a network game needs this build: the original game rejects an options string that has the `CoH` key.

## Install

Run this from any shell. It sets up the Visual Studio x86 environment itself.

```powershell
CoH\scripts\install.ps1
```

The script does four things:
1. If Zero Hour is running, it asks you to close it first. It never closes the game itself.
2. It builds the `win32-profile` preset with `--config Release` and checks that `generalszh.exe` is newer than every source file. Without `--config Release` the multi-config build only builds Debug.
3. It copies `generalszh.exe` and `generalszh.pdb` into `C:\Program Files (x86)\Steam\steamapps\common\Command & Conquer Generals - Zero Hour`.
4. It replaces `CoH\Packs` in the game folder with the `Data\INI` pack of every feature in the build. A pack only loads in games where its flag is on.

`-NoBuild` installs the last build, after the same timestamp check.

## Testing

- **Unit tests:** configure with `-DRTS_BUILD_OPTION_TESTS=ON`, then build and run `z_googletest`, or use `ctest`. Every `CoH/**/Tests/*.cpp` builds into `z_googletest`.
  ```powershell
  cmake --preset win32-profile -DRTS_BUILD_OPTION_TESTS=ON
  cmake --build build/win32-profile --config Release --target z_googletest
  build\win32-profile\Tests\Google\Release\z_googletest.exe --gtest_filter=Feature*:GameFlags.*:Arguments.*
  ```
- **Quick skirmish:** one AI on a small map, with the given flags.
  ```powershell
  CoH\scripts\skirmish.ps1 -Flags Sectors
  CoH\scripts\skirmish.ps1 -Flags Sectors,Accuracy -Map 'Tournament Island' -AI hard -Side FactionGLA -Windowed
  ```
  The script uses the Kernel's command-line options `-cohSkirmish [map]`, `-cohSide <PlayerTemplate>` and `-cohAI easy|medium|hard`. They also work with `-headless`.
- **Replay check:** plays replays headless and reports mismatches. It plays `Replays\CoH\*.rep` by default.
  ```powershell
  CoH\scripts\replaycheck.ps1
  ```
  `Replays\CoH` holds two replays recorded with every flag off. `main` plays both without a mismatch. Retail replays from `GeneralsReplays` need a VC6 build (see TESTING.md). The `win32-profile` build of `main` mismatches every one of them at frame 110, and this build does exactly the same.

## Writing a feature

1. Make `Features/<Name>/` with the layout in PLAN.md.
2. Add `coh_add_feature(<Name>)` to `CoH/CMakeLists.txt`. Its sources build into the engine, its `Tests/*.cpp` build into `z_googletest`, and the Kernel registers it. Packs load in the order of these lines.
3. Declare the slice once in its `Engine/` code:
   ```cpp
   #include "Kernel/Kernel.h"

   namespace
   {
   void onLogicFrame() { /* ... */ }

   void init(CoH::Feature &self)	// once, at engine init
   {
   	CoH::Hooks::logicFrame().subscribe(self, onLogicFrame);
   	CoH::registerIniBlock("CoHSectors", parseSectorsBlock);
   	COH_ADD_MODULE(SectorPointUpdate);
   }

   void reset() { /* after every game: per-game state and INI tuning back to defaults */ }
   }

   COH_FEATURE(Sectors, init, reset)
   ```
4. Put the INI pack in `Data/INI/`. It loads after the base INI and before `map.ini`, only while the flag is on, and it is unloaded between games. It loads like a `map.ini` does (`INI_LOAD_CREATE_OVERRIDES`), so it can change and add objects, command sets, weapons and the other blocks a `map.ini` can.

A subscriber only runs while its feature is on.

## Hook hub

These are the points of `Kernel/Hooks.h` that features subscribe to:

| Event | When |
|---|---|
| `gameStartBeforeMap(loadingSaveGame)` | A game starts. The active packs are loaded; `map.ini` and the map are not. |
| `gameStartAfterMap(loadingSaveGame)` | The map, players and starting units are in. For a save, its objects load after this. |
| `logicFrame()` | Every logic frame, after the objects update and before destroyed objects are removed. |
| `objectCreated(Object *)` | An object is fully set up (`ThingFactory::newObject`). |
| `objectDestroyed(Object *)` | An object is marked destroyed (`GameLogic::destroyObject`). |
| `gameEnd()` | Every object is gone. The feature's `reset` runs next. |

Features register their modules (`COH_ADD_MODULE`) and INI blocks (`CoH::registerIniBlock`) in `init`. The Kernel passes them on through the module factory hook and the INI block table hook.

When a feature needs a new point, it adds one general point to the hub, named after the engine event, and lists its call site below.

## Hooks in original code

Every edit to an original file is a minimal call marked `// CoH hook` (`# CoH hook` in CMake). Edits in `Core/` are wrapped in `#if RTS_ZEROHOUR`. Each file that calls the hub also includes `Kernel/Hooks.h`, and that include line is marked the same way.

| File | Where | Hook | Why |
|---|---|---|---|
| `CMakeLists.txt` | after `Tests` | `add_subdirectory(CoH)` | Builds the CoH target after the targets it adds to. |
| `GeneralsMD/Code/GameEngine/CMakeLists.txt` | `z_gameengine` | links `zi_coh` | Compiles CoH into the engine. |
| `GeneralsMD/.../Common/GameEngine.cpp` | `GameEngine::init` | `onEngineInit()` | Registers the features and reads the local flags, before the module factory starts. |
| `GeneralsMD/.../GameClient/GameClient.cpp` | `GameClient::update` | `onShellShown()` | Starts `-cohSkirmish` once the intro is over and the shell is up, where the engine starts `-loadsave`. |
| `GeneralsMD/.../Common/Thing/ModuleFactory.cpp` | `ModuleFactory::init` | `onModuleFactoryInit(*this)` | Adds the features' modules. |
| `Core/.../Common/INI/INI.cpp` | `findBlockParse` | `findIniBlock(token)` | Finds the features' INI blocks. |
| `GeneralsMD/.../Common/System/SaveGame/GameState.cpp` | `GameState::init` | `addSnapshotBlock("CHUNK_CoH", saveBlock())` | Saves the flags before the map, so a load knows them first. |
| `GeneralsMD/.../GameLogic/System/GameLogic.cpp` | `tryStartNewGame` | `onGameStartBeforeMap(loadingSaveGame)` | Picks the game's flags and loads the packs, before `map.ini`. |
| `GeneralsMD/.../GameLogic/System/GameLogic.cpp` | `tryStartNewGame` | `onGameStartAfterMap(loadingSaveGame)` | The map, players and starting units are in. |
| `GeneralsMD/.../GameLogic/System/GameLogic.cpp` | `GameLogic::update` | `onLogicFrame()` | The logic frame. |
| `GeneralsMD/.../GameLogic/System/GameLogic.cpp` | `GameLogic::destroyObject` | `onObjectDestroyed(obj)` | An object is destroyed. |
| `GeneralsMD/.../GameLogic/System/GameLogic.cpp` | `GameLogic::reset` | `onGameEnd()` | The game is over: features reset and turn off. |
| `GeneralsMD/.../Common/Thing/ThingFactory.cpp` | `ThingFactory::newObject` | `onObjectCreated(obj)` | An object is created. |
| `Core/.../GameNetwork/GameInfo.cpp` | `buildGameInfoAsciiString` | `gameOptions(&game)` | Writes the host's flags into the game options and replay header. Writes nothing when no flag is on. |
| `Core/.../GameNetwork/GameInfo.cpp` | `ParseAsciiStringToGameInfo` | `isGameOptionKey(key)` | Accepts the `CoH` key. |
| `Core/.../GameNetwork/GameInfo.cpp` | `ParseAsciiStringToGameInfo` | `onGameOptionsParsed(game, options)` | Keeps the flags a host or replay sent. |

There are no `// CoH fix` edits yet.
