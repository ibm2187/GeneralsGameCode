# 00 Kernel

**Branch** `coh/kernel` · **Needs** none · **Slice** `CoH/Kernel/`

## Goal

Build the base every feature stands on, so feature tasks only add slices and subscribe to hooks.

## Scope

- **Build:** `CoH/CMakeLists.txt` with one target compiled into `z_gameengine`.
  Original hooks: `add_subdirectory(CoH)` in the root `CMakeLists.txt`, and linking the target in `GeneralsMD/Code/GameEngine/CMakeLists.txt`.
  A feature adds its own sources by listing its folder; the Kernel needs no change for that.
- **Feature registry:** each slice registers once (name, flag, INI pack folder, init and reset functions). No central switch statement.
- **Flags:**
  - Local flags come from `Features.ini` in the user data folder and the command line (`-coh Sectors,Accuracy`).
  - The host's flags are sent in the game options, so every player uses the same set.
  - Flags are saved with skirmish saves and replays, and a replay plays back with the flags it was recorded with.
- **INI packs:** a feature's `Data/INI` files load after the base INI and before `map.ini`, only while its flag is on, and are unloaded between games.
  A pack can override objects, command sets, weapons and so on.
- **Hook hub** (`Kernel/Hooks.h`): one place that original code calls and features subscribe to.
  Start with the lifecycle points: engine init, game start (before and after the map loads), logic frame, object created, object destroyed, game end.
  Later tasks add points as they need them.
  One hook in `ModuleFactory::init` lets features register their own modules. One hook in the INI block table lets features register their own INI blocks.
- **Tests:** `CoH/**/Tests/*.cpp` builds into `z_googletest` with `RTS_BUILD_OPTION_TESTS`. Include a sample test for the registry.
- **Install script** `CoH/scripts/install.ps1`:
  - Builds the profile preset with `--config Release`, otherwise the exe silently stays stale; check the exe's timestamp.
  - Copies `generalszh.exe` and `.pdb` and the enabled packs into the Steam Zero Hour folder.
  - If the game is running, it asks before doing anything; it never closes the game on its own.
- **Quick test:** `CoH/scripts/skirmish.ps1` starts a skirmish against the AI on a small map with given flags, for play-testing.
- **Docs:** `CoH/README.md` with the feature table, install steps and the hooks table.

## Done

- The engine builds and plays exactly like `main` with no flags on (a retail replay still passes).
- A dummy flag can be turned on, its pack loads, and its frame hook runs (removed again before the merge).
- Registry tests pass. Install and quick-test scripts work.
- Merged into `coh/main`.
