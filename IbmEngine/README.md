# IBM Engine

This fork's own features, kept apart from the original engine code so upstream merges stay easy.

Each feature is a vertical slice under `Features/<Name>/`, holding everything for that feature: its C++, its INI pack (`Data/`) and a README. The C++ compiles into the Zero Hour engine library (`z_gameengine`), the way Core's sources do.

| Slice | What it does |
|---|---|
| [FeatureFlags](Features/FeatureFlags/README.md) | Feature flags, INI packs loaded at game start, and the `ObjectCopy` INI block |
| [Squads](Features/Squads/README.md) | Infantry squads and commando squads that heal and refill at the Barracks (flags `Squads`, `SquadsDemo`) |

## Install

```powershell
.\IbmEngine\scripts\install.ps1                      # profile build -> generalszh.exe, plus every feature pack
.\IbmEngine\scripts\install.ps1 -Preset win32-debug  # debug build   -> generalszh_dbg.exe
.\IbmEngine\scripts\install.ps1 -Enable Squads       # also switch a feature on in Features.ini
```

## Hooks in original code

Every change to original engine files is small and marked `// IBM Engine hook` (`# IBM Engine hook` in CMake). Search for that text to find them all.

| File | Hook |
|---|---|
| `CMakeLists.txt` | `add_subdirectory(IbmEngine)` |
| `GeneralsMD/Code/GameEngine/CMakeLists.txt` | links `ibm_engine` into `z_gameengine` |
| `GeneralsMD/Code/GameEngine/Source/Common/GameEngine.cpp` | `IbmEngine::initFeatureFlags()` after the command line is parsed |
| `GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp` | `IbmEngine::onStartNewGame()` just before `map.ini` loads |
| `Core/GameEngine/Include/GameNetwork/GameInfo.h` | `m_featureFlags` with getter and setter |
| `Core/GameEngine/Source/GameNetwork/GameInfo.cpp` | flags reset to local flags; written as `FF=` in the game options only when non-zero; parsed back |
| `Core/GameEngine/Source/Common/INI/INI.cpp` | registers the `ObjectCopy` INI block |
| `GeneralsMD/Code/GameEngine/Source/GameClient/GUI/GUICallbacks/Menus/SkirmishGameOptionsMenu.cpp` | a skirmish uses the current local flags, not the ones saved with the last skirmish's settings |
| `Core/GameEngine/Include/GameClient/ControlBar.h` | `postProcessCommands()` made public, so packs can resolve the images of buttons they add |
| `GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Locomotor.cpp` | fix: `LocomotorStore::reset` used an erased iterator, crashing at game end when an override INI added a locomotor |
| `GeneralsMD/Code/GameEngine/Source/GameLogic/System/GameLogic.cpp` | `IbmEngine::setUpSquadsDemo()` after the starting bases are placed (not when loading a save) |
| `GeneralsMD/Code/GameEngine/Source/Common/Thing/ModuleFactory.cpp` | registers the `IbmSquadUpdate` module |
| `GeneralsMD/Code/GameEngine/Include/GameLogic/Module/SpawnBehavior.h`, `.../Behavior/SpawnBehavior.cpp` | `getSlaveIDs()` and `adoptSlave()`: squads read their members and take back the ones the Barracks returns |
| `GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/MobMemberSlavedUpdate.cpp` | a squad member using a skill or riding in a container isn't pulled back to its leader |
| `GeneralsMD/Code/GameEngine/Source/Common/RTS/ActionManager.cpp` | `canEnterObject`: squads may enter containers |
| `GeneralsMD/Code/GameEngine/Source/GameLogic/Object/Update/AIUpdate.cpp`, `Include/.../AIUpdate.h` | enter and exit: whole squads; attack and force attack on a trooper attack the whole squad (`privateAttackSquadOf`) |
| `GeneralsMD/Code/GameEngine/Include/GameLogic/AIStateMachine.h`, `Source/GameLogic/AI/AIStates.cpp` | new state `AI_FORCE_ATTACK_SQUAD` (appended, so saved state ids don't change); attack-squad victims spread over a squad |
| `GeneralsMD/Code/GameEngine/Source/Common/RTS/Team.cpp` | `TeamTemplateInfo`: AI teams build squads instead of basic infantry |
| `Core/GameEngine/Source/GameLogic/System/GameLogicDispatch.cpp` | skill orders given to a squad go to the member with the skill |
| `Core/GameEngine/Source/GameClient/GUI/ControlBar/ControlBarCommand.cpp` | a squad's skill buttons are evaluated for the member with the skill |
| `GeneralsMD/Code/GameEngine/Source/Common/RTS/SpecialPower.cpp` | fix: `SpecialPowerStore::reset` gave a special power from a pack or map.ini a new ID every game, so replays of later games couldn't find it |
| `Core/GameEngineDevice/Source/MilesAudioDevice/MilesAudioManager.cpp` | fix: the audio file cache held its lock while stopping a sound whose Miles callback needed it, freezing the game ([upstream #256](https://github.com/TheSuperHackers/GeneralsGameCode/issues/256)) |

Hooks in `Core/` are wrapped in `#if RTS_ZEROHOUR`, because Core is also compiled into the original Generals, which doesn't link this library.
