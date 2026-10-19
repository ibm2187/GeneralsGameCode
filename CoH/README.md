# CoH

Company of Heroes ideas for Zero Hour, kept apart from the original code. [PLAN.md](Docs/PLAN.md) has the direction, the conventions and the roadmap; [Tasks](Docs/Tasks) has one file per feature; [CONTEXT.md](Docs/CONTEXT.md) maps what to read for which job.

Everything lives in this folder, in main packages:
- `Kernel/` is shared by every feature: the feature registry, the flags, the INI packs and the hook hub.
- `Features/<Name>/` holds one vertical slice per feature.
- `Scripts/` builds, installs, packages and tests: `install.ps1`, `package.ps1`, `skirmish.ps1`, `replaycheck.ps1`.
- `Tools/` holds developer tools: `retail.py` writes the retail data to `build/retail/`.
- `Docs/` holds the plan, the context map and the task files.

The original code only calls the hook hub, from the places listed in [Hooks in original code](#hooks-in-original-code).

## Features

| Feature | Flag | Task | Status |
|---|---|---|---|
| [Sectors](Features/Sectors/README.md) | `Sectors` | [01](Docs/Tasks/01-Sectors.md) | supplies only from capturable sectors; skirmish AI plays for them |
| [GeneralsArsenal](Features/GeneralsArsenal/README.md) | `GeneralsArsenal` | [02](Docs/Tasks/02-GeneralsArsenal.md) | jets and super weapons are General's powers paid with command points earned in combat; helicopters at the Airfield; skirmish AI uses them |
| [Upkeep](Features/Upkeep/README.md) | `Upkeep` | [03](Docs/Tasks/03-Upkeep.md) | units take population under a cap that grows with buildings and sectors; upkeep takes a growing share of income; skirmish AI keeps its upkeep low (needs Sectors) |
| [Squads](Features/Squads/README.md) | `Squads` | [09](Docs/Tasks/09-Squads.md) | infantry is ordered, selected, carried and counted as squads that move in formation, with one health bar; skirmish AI builds and orders them |
| [Accuracy](Features/Accuracy/README.md) | `Accuracy` | [04](Docs/Tasks/04-Accuracy.md) | direct fire rolls to hit by weapon type and range, target size (troops hardest, buildings easiest), movement and veterancy; a miss lands beside the target; other slices add hit modifiers through `Accuracy.h` |
| [FireRate](Features/FireRate/README.md) | `FireRate` | [04b](Docs/Tasks/04b-FireRate.md) | fast weapons fire about as often as in CoH and hit harder by the same factor, so damage per second stays the same but each shot (and Accuracy roll) counts |
| [Pacing](Features/Pacing/README.md) | `Pacing` | [03b](Docs/Tasks/03b-Pacing.md) | units cost more and take longer to build, by class or by unit; the skirmish AI's income and build times scale with its difficulty |
| [DirectionalArmor](Features/DirectionalArmor/README.md) | `DirectionalArmor` | [07](Docs/Tasks/07-DirectionalArmor.md) | vehicles have front, side and rear armor and weapons penetration by range; a shot that doesn't penetrate deflects with sparks and a clang; rockets mostly bounce off a tank's front; tank guns and vehicle machine guns shred infantry; skirmish AI flanks vehicles |
| [UnitScale](Features/UnitScale/README.md) | `UnitScale` | [19](Docs/Tasks/19-UnitScale.md) | vehicles drawn bigger and infantry smaller, in CoH proportions, with the geometry to match (INI pack) |

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
| A save game | the flags it was saved with (the `CHUNK_CoH` save block); the features' own state is in `CHUNK_CoHFeatures` |

With no flags on, the game options string is exactly the original, so replays recorded with no flags stay compatible with `main`. Every player of a network game needs this build: the original game rejects an options string that has the `CoH` key.

## Install

Run this from any shell. It sets up the Visual Studio x86 environment itself.

```powershell
CoH\Scripts\install.ps1
```

The script does four things:
1. If Zero Hour is running, it asks you to close it first. It never closes the game itself.
2. It builds the `win32-profile` preset with `--config Release` and checks that `generalszh.exe` is newer than every source file. Without `--config Release` the multi-config build only builds Debug.
3. It copies `generalszh.exe` and `generalszh.pdb` into `C:\Program Files (x86)\Steam\steamapps\common\Command & Conquer Generals - Zero Hour`.
4. It replaces `CoH\Packs` in the game folder with the `Data\INI` pack of every feature in the build. A pack only loads in games where its flag is on.

`-NoBuild` installs the last build, after the same timestamp check.

## Release

Players get a zip they unzip into their Zero Hour folder; they never build anything.

```powershell
CoH\Scripts\package.ps1 *> $log
CoH\Scripts\package.ps1 -NoBuild -Version 0.1 -WithSymbols
```

It builds like `install.ps1` (or takes `-NoBuild`) and writes `build\release\CoH-<version>.zip`, with its files at the top so it unzips next to the retail files:

| File | What |
|---|---|
| `generalszh.exe` | the build; `generalszh.pdb` too with `-WithSymbols` |
| `msvcp140.dll`, `vcruntime140.dll`, ... | the Visual C++ runtime DLLs the exe imports, from the building Visual Studio's redistributable folder, so players install no runtime. The exe needs a runtime at least as new as its compiler |
| `CoH\Packs\<Name>\` | the `Data\INI` pack of every feature in the build |
| `Features.ini.default` | every feature on. The game reads `Features.ini` only from the user data folder, so `README-CoH.txt` tells the host to copy it there; players who only join need no `Features.ini` (the host's flags apply) |
| `README-CoH.txt` | what it is, install, multiplayer (same release for everyone, the host's flags, no retail or GenTool players), turning features off, the source link |
| `LICENSE.txt` | GPL v3 |

- The version is `-Version`, or else `git describe --tags --always --dirty --match coh-v*` without the `coh-v`: a commit hash until there is a `coh-v` tag.
- The README links to the source at the exact commit, from `git remote get-url origin`. The script warns when the commit isn't on a remote branch or the tree has changes, since the link would then not show the build.
- No EA data goes in: every file is checked against the allowed types (`.ini` only in the packs) and the script stops on anything else.
- The `CoH Release` workflow (`.github/workflows/coh-release.yml`) does the same on GitHub when a `coh-v*` tag is pushed (`coh-v0.1` makes `CoH-0.1.zip`): it builds `win32-profile`, runs `package.ps1 -NoBuild` and attaches the zip to a release for that tag.

## Testing

- **Unit tests:** configure with `-DRTS_BUILD_OPTION_TESTS=ON`, then build and run `z_googletest`, or use `ctest`. Every `CoH/**/Tests/*.cpp` builds into `z_googletest`.
  ```powershell
  cmake --preset win32-profile -DRTS_BUILD_OPTION_TESTS=ON
  cmake --build build/win32-profile --config Release --target z_googletest
  build\win32-profile\Tests\Google\Release\z_googletest.exe --gtest_filter=Feature*:GameFlags.*:Arguments.*
  ```
- **Quick skirmish:** one AI on a small map, with the given flags.
  ```powershell
  CoH\Scripts\skirmish.ps1 -Flags Sectors
  CoH\Scripts\skirmish.ps1 -Flags Sectors,Accuracy -Map 'Tournament Island' -AI hard -Side FactionGLA -Windowed
  ```
  The script uses the Kernel's command-line options `-cohSkirmish [map]`, `-cohSide <PlayerTemplate>` and `-cohAI easy|medium|hard`. They also work with `-headless`.
- **Unattended AI games:** `-cohSide Observer -cohOpponents 2` watches two AIs, `-cohOpponentSides FactionChinaNukeGeneral,FactionGLA` picks their sides (one not given is random), `-cohQuitAfter <seconds>` quits after that much game time or when the game is over, several quick skirmishes may run at once (each writes its own `CoH_InstanceNN.log` and replay; `-cohSeveralGames` lets another test run, such as `-loadsave`, start beside them), `-cohLog` makes features write what happens to `CoH.log` in the user data folder (the profile build has no debug log; `-cohLog <name>` names the file, for games run side by side), and `-cohStats [seconds]` adds each side's income, units built and army to it every minute (or every that many seconds), with any flags, retail included. Run them from the game folder, headless, so no window opens:
  ```powershell
  generalszh.exe -useCwd -headless -coh Sectors -cohSkirmish "Tournament Desert" -cohSide Observer -cohOpponents 2 -cohAI hard -cohLog -cohQuitAfter 600
  ```
- **Replay check:** plays replays headless and reports mismatches. It plays `Replays\CoH\*.rep` by default.
  ```powershell
  CoH\Scripts\replaycheck.ps1
  ```
  `Replays\CoH` holds two replays recorded with every flag off. `main` plays both without a mismatch. Retail replays from `GeneralsReplays` need a VC6 build (see TESTING.md). The `win32-profile` build of `main` mismatches every one of them at frame 110, and this build does exactly the same.

## Writing a feature

1. Make `Features/<Name>/` with the layout in Docs/PLAN.md.
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
4. Put the INI pack in `Data/INI/`. It loads after the base INI and before `map.ini`, only while the flag is on, and it is unloaded between games. It loads like a `map.ini` does (`INI_LOAD_CREATE_OVERRIDES`), so it can change and add objects, command sets, weapons and the other blocks a `map.ini` can. Its files load in name order, and a block must come after what it names (an OCL before the module that uses it). A new object lists its modules inside `AddModule`, and a `Locomotor` line goes after its AI module.
5. State a feature keeps between frames that isn't in its modules (a list of squads, say) is saved with `CoH::registerSaveData(self, "<Name>", snapshot)` in `init`. The Kernel saves each feature's snapshot, while the feature is on, in its own named block of `CHUNK_CoHFeatures`, which comes after the objects (`CHUNK_GameLogic`): on load the snapshot's `xfer` runs when every object has its saved ID again, so it may look objects up by ID. A save without the feature's data (an older save, or one made with the flag off) skips it, and the feature starts as in a new game; data of a feature that isn't on is skipped. Version the snapshot's data (`xferVersion`) so older saves keep loading. Modules keep saving themselves as the engine's do.
6. Text for labels the game's string file lacks (a new button's name and tooltip) goes in a `CoHText` block of the pack, one `Text = <label> "<text>"` line each (`
` starts a new line). The game's own text wins; pack text is forgotten when the game ends.

A subscriber only runs while its feature is on.

### In C++20, against the Kernel wrapper

New code is written in modern C++20 against the Kernel's wrapper headers (`Docs/PLAN.md`, *Conventions* 10). Existing slices keep their style.

| Header | For |
|---|---|
| `Kernel/Time.h` | `now()`, `everySeconds(s)`, `secondsFromNow(s)`, `framesFromSeconds(s)`, `isEvery(frame, period)` |
| `Kernel/Random.h` | synced `chance(p)`, `pick(items)`, `between(lo, hi)` |
| `Kernel/Objects.h` | `objectsNear(center, radius, filter)`, `objectsOf(player, filter)`, `allObjects(filter)`, `kinds(...)`, `findObject(id)`, `findLiving(id)` |
| `Kernel/Players.h` | `players()`, `sides()`, `activeSides()`, `aiSides()`; `isEnemy`, `isAlly` |
| `Kernel/Ini.h` | `Ini::Table` of `Ini::field`, `Ini::seconds`, `Ini::percent`, `Ini::custom` |

A whole feature, *FieldAid*: every two seconds, infantry near a friendly heal pad may heal a little.
`Kernel/Tests/WrapperExampleTest.cpp` compiles this code, so it stays correct. In a real slice the heal rule would be a pure function in `Domain/` with a test.

```cpp
// Features/FieldAid/Engine/FieldAid.cpp
#include "PreRTS.h"	// This must go first in EVERY cpp file in the GameEngine

#include "GameLogic/Object.h"

#include "Kernel/Ini.h"
#include "Kernel/Kernel.h"
#include "Kernel/Objects.h"
#include "Kernel/Players.h"
#include "Kernel/Random.h"
#include "Kernel/Time.h"

namespace CoH::FieldAid
{

namespace
{

struct Config	// the CoHFieldAid block of Data/INI/FieldAid.ini; these are the defaults
{
	Real radius = 150.0f;
	Real healChance = 0.5f;
	Real healAmount = 10.0f;
	UnsignedInt intervalFrames = framesFromSeconds(2.0f);
	KindOfMaskType medicKinds = kinds(KINDOF_HEAL_PAD);
};

Config &config()
{
	static Config theConfig;
	return theConfig;
}

void parseConfigBlock(INI *ini)
{
	static const Ini::Table<Config> fields = {
		Ini::field("Radius", &Config::radius),
		Ini::percent("HealChance", &Config::healChance),	// "50%"
		Ini::field("HealAmount", &Config::healAmount),
		Ini::seconds("Interval", &Config::intervalFrames),	// "2.0", kept as 60 frames
		Ini::field("MedicKinds", &Config::medicKinds),
	};
	fields.read(ini, config());
}

void onLogicFrame()
{
	const Config &settings = config();
	if (!isEvery(now(), settings.intervalFrames))
		return;
	for (Player *player : activeSides())
	{
		for (Object *medic : objectsOf(*player, { .anyKind = settings.medicKinds }))
		{
			for (Object *patient : objectsNear(*medic, settings.radius, { .alliesOf = medic, .anyKind = kinds(KINDOF_INFANTRY) }))
			{
				if (chance(settings.healChance))	// synced: one draw per patient, in the engine's order
					patient->attemptHealing(settings.healAmount, medic);
			}
		}
	}
}

void init(Feature &self)
{
	registerIniBlock("CoHFieldAid", parseConfigBlock);
	Hooks::logicFrame().subscribe(self, onLogicFrame);
}

void reset()
{
	config() = Config();
}

} // namespace

} // namespace CoH::FieldAid

COH_FEATURE(FieldAid, CoH::FieldAid::init, CoH::FieldAid::reset)
```

The range from `objectsNear` deletes the engine's iterator itself. A module's data uses the same tables: `p.add(fields)` in its `buildFieldParse`.

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
| `buildLocation()` | A `Veto`: may this structure go at this spot for this player? Asked for the placement cursor and the AI's base building. Allowed unless an active feature says no. |
| `inGameUIPostDraw()` | Draw over the 3D view (`TheDisplay`, screen pixels), under the windows. Not in `-headless` games. |
| `radarDraw(x, y, width, height)` | Draw on the minimap: the area its map is drawn in, over the terrain and shroud, under the icons. |
| `moneyTooltip(player, description)` | The money tooltip is being made for the viewed player; add to its text. |
| `killScored(killer, victim)` | A kill of an enemy is credited (`Object::scoreTheKill`, as skill points are). |
| `damageDealt(victim, damage)` | An object lost health; `damage->out.m_actualDamageClipped` is how much, `in.m_sourceID` who did it. |
| `specialPowerUse(source, power)` | A `Veto`: may this object use this special power now? Asked when a power is used (not when a script forces it) and for its button, which a no greys out. |
| `specialPowerTriggered(source, power, target)` | A special power fired and its recharge started; the target may be null. |
| `commandTooltip(button, description)` | A command button's tooltip is being made; add to its text. |
| `moneyDisplay(player, text)` | The money text is being made ("$ 1200"); add to it. With a subscriber on, the hub writes it every frame (the per-minute income display is replaced). |
| `unitProduced(factory, unit)` | A factory made a unit; it has its team and is on its way out of the door. Raised for each unit of an order that makes several. |
| `buildCost(player, build, cost)`, `buildTime(player, build, frames)` | What a unit or structure costs the player and how many frames it takes, with the player's modifiers; a subscriber may change the amount. Asked for buttons, tooltips, the queue, refunds and the AI. |
| `playerSelection(player, object, selected)` | A player's selection message selected or deselected one of its objects (clicks, boxes). Not deaths or scripts. |
| `aiCommand(object, command)` | An object accepted a command (`AICommandParms`: move, attack, enter, exit, ...) from a player, a script or the AI. |
| `containRoom(container, rider, used, needed)` | May the rider go in? `used` is the room taken and `needed` what the rider takes (transport slots, garrison places); it fits while `used + needed` is at most the maximum. A subscriber may change both. |
| `weaponBonus(source, weapon, bonus)` | The bonuses a weapon fires with were summed (`Weapon::computeBonus`): damage, radius, range, rate of fire, pre-attack delay. A subscriber may change them; they apply to shots, reloads, a projectile going off (the projectile is then the source) and the AI's estimates. |
| `healthBar(object, bar, health, maxHealth)` | A selected or moused-over object's health bar is drawn in `bar` from these; a subscriber may change them, and a max of 0 draws none. |
| `weaponFiringAtObject(shot)` | A weapon is about to fire at an object (not a projectile going off). A subscriber may send the shot to a spot instead (a miss) by writing `*shot.spot` and setting `shot.retargeted`; the engine then fires at that spot as at the ground, so tracers, projectiles and blast damage go there. The first to retarget wins. |
| `weaponDetonated(source, weapon, position, radius)` | A weapon's damage goes off at a spot (a shot landing, a projectile going off), before anything is hurt; `radius` is its blast. |
| `weaponDamage(hit)` | A weapon's damage is about to be dealt to one victim (`hit.victim`, `hit.damage`, where it went off in `hit.position`). A subscriber may change `hit.damage->in.m_amount`. A weapon killing its user comes as `HUGE_DAMAGE_AMOUNT`. Subscribers run in feature order: Cover, then DirectionalArmor. |
| `riderShown(container, rider)` | A `Veto`: does this rider get a passenger button of its own on the container's command bar (a transport's or a garrison's) and a pip over it? A no hides it, because another rider speaks for it; each hidden rider takes one pip less. |

The hub also passes on drawing that `TheDisplay` doesn't offer, which original code provides:

| Function | What |
|---|---|
| `drawGradientQuad(corners, color1, color2)` | A filled four-cornered shape in screen pixels, shaded from `color1` at corners 0 and 1 to `color2` at corners 2 and 3 (`Render2DClass::Add_Quad_HGradient`). Call it while drawing (`inGameUIPostDraw`, `radarDraw`); it is batched with `TheDisplay`'s lines. Returns false where no display provides it. |

A `Veto` is a question with a yes/no answer (`subscribe` a function returning `bool`); an `Event` only tells. `CoH::log(...)` (`Kernel/Log.h`) writes to `CoH.log` with `-cohLog`.

Features register their modules (`COH_ADD_MODULE`) and INI blocks (`CoH::registerIniBlock`) in `init`. The Kernel passes them on through the module factory hook and the INI block table hook.

When a feature needs a new point, it adds one general point to the hub, named after the engine event, and lists its call site below.

## Hooks in original code

Every edit to an original file is a minimal call marked `// CoH hook` (`# CoH hook` in CMake). Edits in `Core/` are wrapped in `#if RTS_ZEROHOUR`. Each file that calls the hub also includes `Kernel/Hooks.h`, and that include line is marked the same way.

| File | Where | Hook | Why |
|---|---|---|---|
| `CMakeLists.txt` | after `Tests` | `add_subdirectory(CoH)` | Builds the CoH target after the targets it adds to. |
| `GeneralsMD/Code/GameEngine/CMakeLists.txt` | `z_gameengine` | links `zi_coh` | Compiles CoH into the engine. |
| `Core/.../Common/CommandLine.cpp` | `CommandLine::parseCommandLineForStartup` | `allowsSeveralGames()` | With `-cohSkirmish` or `-cohSeveralGames`, the game may run beside others (multi-instance), as replay playback may. |
| `GeneralsMD/.../Common/GameEngine.cpp` | `GameEngine::init` | `onEngineInit()` | Registers the features and reads the local flags, before the module factory starts. |
| `GeneralsMD/.../GameClient/GameClient.cpp` | `GameClient::update` | `onShellShown()` | Starts `-cohSkirmish` once the intro is over and the shell is up, where the engine starts `-loadsave`. |
| `GeneralsMD/.../Common/Thing/ModuleFactory.cpp` | `ModuleFactory::init` | `onModuleFactoryInit(*this)` | Adds the features' modules. |
| `Core/.../Common/INI/INI.cpp` | `findBlockParse` | `findIniBlock(token)` | Finds the features' INI blocks. |
| `GeneralsMD/.../Common/System/SaveGame/GameState.cpp` | `GameState::init` | `addSnapshotBlock("CHUNK_CoH", saveBlock())` | Saves the flags before the map, so a load knows them first. |
| `GeneralsMD/.../Common/System/SaveGame/GameState.cpp` | `GameState::init` | `addSnapshotBlock("CHUNK_CoHFeatures", featureSaveBlock())` | The features' saved state (`registerSaveData`), right after `CHUNK_GameLogic`, so on load the objects have their saved IDs. The original game skips the block as unknown; a save without it loads as before. |
| `GeneralsMD/.../GameLogic/System/GameLogic.cpp` | `tryStartNewGame` | `onGameStartBeforeMap(loadingSaveGame)` | Picks the game's flags and loads the packs, before `map.ini`. |
| `GeneralsMD/.../GameLogic/System/GameLogic.cpp` | `tryStartNewGame` | `onGameStartAfterMap(loadingSaveGame)` | The map, players and starting units are in. |
| `GeneralsMD/.../GameLogic/System/GameLogic.cpp` | `GameLogic::update` | `onLogicFrame()` | The logic frame. |
| `GeneralsMD/.../GameLogic/System/GameLogic.cpp` | `GameLogic::destroyObject` | `onObjectDestroyed(obj)` | An object is destroyed. |
| `GeneralsMD/.../GameLogic/System/GameLogic.cpp` | `GameLogic::reset` | `onGameEnd()` | The game is over: features reset and turn off. |
| `GeneralsMD/.../Common/Thing/ThingFactory.cpp` | `ThingFactory::newObject` | `onObjectCreated(obj)` | An object is created. |
| `Core/.../GameNetwork/GameInfo.cpp` | `buildGameInfoAsciiString` | `gameOptions(&game)` | Writes the host's flags into the game options and replay header. Writes nothing when no flag is on. |
| `Core/.../GameNetwork/GameInfo.cpp` | `ParseAsciiStringToGameInfo` | `isGameOptionKey(key)` | Accepts the `CoH` key. |
| `Core/.../GameNetwork/GameInfo.cpp` | `ParseAsciiStringToGameInfo` | `onGameOptionsParsed(game, options)` | Keeps the flags a host or replay sent. |
| `GeneralsMD/.../Common/System/BuildAssistant.cpp` | `isLocationLegalToBuild` | `isBuildLocationAllowed(build, pos, player, builder)` | The `buildLocation` veto, after the shroud check; a no is `LBC_RESTRICTED_TERRAIN`. |
| `GeneralsMD/.../GameClient/InGameUI.cpp` | `InGameUI::postDraw` | `onInGameUIPostDraw()` | Drawing over the 3D view. |
| `Core/GameEngineDevice/.../W3DRadar.cpp` | `W3DRadar::draw` | `onRadarDraw(x, y, width, height)` | Drawing on the minimap, after the shroud. |
| `Core/GameEngineDevice/.../W3DDisplay.h`, `W3DDisplay.cpp` | `W3DDisplay::init`, new `W3DDisplay::drawGradientQuad` | `provideGradientQuad(...)` | Gives the hub a gradient-filled quad (`Add_Quad_HGradient`) that `Display` has no call for; used for Sectors' territory bands. |
| `Core/.../GUICallbacks/ControlBarPopupDescription.cpp` | `ControlBar::populateBuildTooltipLayout` | `onMoneyTooltip(player, descrip)` | The money tooltip's text. |
| `GeneralsMD/Code/GameEngineDevice/CMakeLists.txt` | `z_gameenginedevice` | links `zi_coh_hooks` | The hub's header for the radar and display hooks (CoH compiles into `z_gameengine` only). |
| `GeneralsMD/.../GameLogic/Object/Object.cpp` | `Object::scoreTheKill` | `onKillScored(this, victim)` | A kill of an enemy is credited, after the skill points. |
| `GeneralsMD/.../GameLogic/Object/Body/ActiveBody.cpp` | `ActiveBody::attemptDamage` | `onDamageDealt(obj, damageInfo)` | Damage taken, once the actual damage is known. |
| `GeneralsMD/.../Common/RTS/SpecialPower.cpp` | `SpecialPowerStore::canUseSpecialPower` | `isSpecialPowerAllowed(obj, power)` | The `specialPowerUse` veto, after the science check; a no means the power can't be used. |
| `GeneralsMD/.../GameLogic/Object/SpecialPower/SpecialPowerModule.cpp` | `SpecialPowerModule::triggerSpecialPower` | `onSpecialPowerTriggered(object, power, location)` | A power fired, after its recharge started. |
| `Core/.../GUI/ControlBar/ControlBarCommand.cpp` | `ControlBar::getCommandAvailability` | `isSpecialPowerAllowed(obj, power)` | A power's button is greyed (`COMMAND_RESTRICTED`) when the veto says no and it isn't recharging. |
| `Core/.../GUICallbacks/ControlBarPopupDescription.cpp` | `ControlBar::populateBuildTooltipLayout` | `onCommandTooltip(commandButton, descrip)` | A command button's tooltip text. |
| `GeneralsMD/.../GameClient/InGameUI.cpp` | `InGameUI::update` | `onMoneyDisplay(moneyPlayer, moneyWin)` | When it returns true, a feature wrote the money text and the original money text is skipped. |
| `Core/.../GameClient/GameText.cpp` | `GameTextManager::fetch` | `findText(label)` | A label the string file lacks is looked up in the active packs' `CoHText`. |
| `GeneralsMD/.../Include/Common/SpecialPowerType.h`, `Source/Common/RTS/SpecialPower.cpp` | `SpecialPowerType`, its names | `SPECIAL_COH_STRIKE_1..3`, `SPECIAL_COH_COVER_1..2` | Special power types for the features' new General's powers, at the end of the enum so saved values don't change; unused with every flag off. |
| `GeneralsMD/.../GameLogic/AI/AIGroup.cpp` | `AIGroup::groupDoSpecialPower`, `...AtLocation`, `...AtObject` | `isSpecialPowerAllowed(object, power)` | A player's power order asks the `specialPowerUse` veto in game logic too (orders don't go through `canUseSpecialPower`). |
| `GeneralsMD/.../Common/RTS/ActionManager.cpp` | `ActionManager::canDoSpecialPowerAtLocation` | the `SPECIAL_COH_*` cases | The new types aim like the other strike powers (not into black shroud); without them a player's cursor shows "can't" for them. |
| `GeneralsMD/.../Common/RTS/Player.cpp` | `Player::canBuildMoreOfType` | `isProductionAllowed(this, whatToBuild)` | The `production` veto, first; a no is "maxed out" (`CANMAKE_MAXED_OUT_FOR_PLAYER`) for the buttons, the queue and the AI's factory search. |
| `GeneralsMD/.../Common/RTS/Money.cpp` | `Money::deposit` | `onIncome(player, amountToDeposit)` | Tracked income only (not starting cash, sales, refunds); a feature may change the amount before it is deposited and counted. |
| `GeneralsMD/.../GameLogic/Object/Update/ProductionUpdate.cpp` | `ProductionUpdate::update` | `onUnitProduced(creationBuilding, newObj)` | A unit was made, after it left by the door, joined its team (`onUnitCreated`) and its create modules ran; before the entry counts it. |
| `Core/.../Common/Thing/ThingTemplate.cpp` | `ThingTemplate::calcCostToBuild`, `calcTimeToBuild` | `onBuildCost(player, this, cost)`, `onBuildTime(player, this, buildTime)` | The final cost and build time, after the player's modifiers. |
| `Core/.../GameLogic/System/GameLogicDispatch.cpp` | `GameLogic::onCreateSelectedGroup`, `onRemoveFromSelectedGroup` | `onPlayerSelection(msgPlayer, obj, selected)` | After each object a selection message selects or deselects. |
| `GeneralsMD/.../GameLogic/Object/Update/AIUpdate.cpp` | `AIUpdateInterface::aiDoCommand` | `onAiCommand(getObject(), parms)` | A command the object accepted, before it is carried out. |
| `GeneralsMD/.../GameLogic/Object/Contain/TransportContain.cpp`, `GarrisonContain.cpp` | `isValidContainerFor` | `onContainRoom(getObject(), rider, used, needed)` | The capacity check, rewritten as `used + needed <= max` (the garrison's `count < max` is the same with `needed` 1). |
| `GeneralsMD/.../GameLogic/Object/Weapon.cpp` | `Weapon::computeBonus` | `onWeaponBonus(source, m_template, bonus)` | Once the weapon's bonuses are summed, before they are used. |
| `GeneralsMD/.../Include/GameLogic/Weapon.h` | `WeaponTemplate` | new `getMinDelayBetweenShots()`, `getMaxDelayBetweenShots()` | A weapon's fire rate without `getDelayBetweenShots`, which draws a synced random number for a `Min`/`Max` delay. |
| `GeneralsMD/.../GameClient/Drawable.cpp` | `Drawable::drawHealthBar` | `onHealthBar(obj, healthBarRegion, health, maxHealth)` | The health and max health the bar is drawn from. |
| `GeneralsMD/.../GameLogic/Object/Weapon.cpp` | `WeaponTemplate::fireWeaponTemplate` | `onWeaponFiringAtObject(sourceObj, this, bonus, victimObj, victimPosStorage)` | After the range checks, before the fire effects, not for a projectile going off. True turns the shot into a shot at the spot, as the engine's sneaky targeting offset does (`victimObj` null, `victimID` invalid). |
| `GeneralsMD/.../GameLogic/Object/Weapon.cpp` | `WeaponTemplate::dealDamageInternal` | `onWeaponDetonated(source, this, pos, radius)` | Once the blast radius is known, before the victims are found (also for a weapon killing its user). |
| `GeneralsMD/.../GameLogic/Object/Weapon.cpp` | `WeaponTemplate::dealDamageInternal` | `onWeaponDamage(source, this, pos, curVictim, &damageInfo)` | Before each victim's `attemptDamage`, once the amount and the launcher are set; a subscriber may change the amount. |
| `Core/.../GUI/ControlBar/ControlBarCommand.cpp` | `ControlBar::populateInvDataCallback` | `isRiderShown(data->transport, obj)` | The `riderShown` veto: a hidden rider gets no transport passenger button; the next rider takes the next button. |
| `Core/.../GUI/ControlBar/ControlBarStructureInventory.cpp` | `ControlBar::populateButtonProc` | `isRiderShown(info->source, obj)` | The same for a garrisoned building's passenger buttons. |
| `GeneralsMD/.../GameClient/Drawable.cpp` | `Drawable::drawContained` | `isRiderShown(obj, *it)` | A hidden rider takes one pip less over its container. |

## Fixes in original code

Each is marked `// CoH fix`, and wrapped in `#if RTS_ZEROHOUR` in `Core/`. With no flag on no pack loads, and a single override (a map.ini's) behaves exactly as before; only a map.ini that overrides the same command set twice, or names `None`, would load differently. The recorder fix changes replay playback only.

| File | Where | Fix |
|---|---|---|
| `Core/.../GUI/ControlBar/ControlBar.cpp` | `ControlBar::newCommandSetOverride` | A second override of a command set (a second pack, or a map.ini after a pack) copied the base set and was chained in front of the first, so only the first override counted. It now builds on the last override and goes after it, as `ThingFactory::newOverride` does for objects. Packs that override the same command set add up, button by button. |
| `GeneralsMD/.../Common/RTS/SpecialPower.cpp` | `SpecialPowerStore::reset` | Special powers a pack (or a map.ini) adds took new IDs every game, since the counter was never wound back, so the same power had another ID in a replay or on another machine. The counter now restarts after the base powers, as `ThingFactory::reset` already does for object templates. |
| `Core/.../GUI/ControlBar/ControlBar.cpp` | `CommandSet::parseCommandButton` | In an override file, `N = None` empties slot N, so a pack can take a button out without restating the set. |
| `GeneralsMD/.../Common/Recorder.cpp` | `RecorderClass::updatePlayback` | A `-replay` playback appended its frame 0 commands in the update that was still starting the game, so they ran before the game existed; a recording runs them after the frame 0 CRC. Every game records one (`MSG_ENABLE_RETALIATION_MODE`), and its AI group took an ID ahead of the frame 0 groups, so a replay with a group alive at the frame 0 CRC (America's Superweapon and Laser generals' scripts) mismatched at frame 0, flags on or off. The playback now appends nothing while its `MSG_NEW_GAME` waits, as one started from the shell already did. Only playback changes; games and recordings are as before. |
