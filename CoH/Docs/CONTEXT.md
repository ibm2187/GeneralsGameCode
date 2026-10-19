# CoH context map

The engine is about 1.5 million lines. A session that explores it freely fills its context before it writes any code.
This map says what to read for each job, so a session opens only the files it needs, at the lines it needs.
**Keep it current.** When a task finds a new place in the engine that later tasks will need, add one line here before merging.

## 1. Start of a task: read these, in order

1. This file.
2. `CoH/Docs/Tasks/<NN>-<Name>.md`, your task.
3. `CoH/Docs/PLAN.md`: only *Conventions* and *Done, for every task*.
4. The README and public header of each slice your task uses. Read the header first; read the README only if the header isn't enough.
5. `CoH/README.md`: only when you add a hook or register something (*Writing a feature*, the hook events, the hooks table).
   It is long, so jump to the section you need.

Don't read other slices' `Engine/` or `Domain/` code to learn the Kernel. Their README and `Kernel/Kernel.h` and `Kernel/Hooks.h` are the interface.
New code is C++20 against the Kernel wrapper (`PLAN.md`, *Conventions* 10). Its headers (§3) are its documentation, and `CoH/README.md` has an example feature.

## 2. Context budget rules

- **Never read a whole engine file.** Many are 3,000 to 10,000 lines (sizes in §4).
  Grep for the function with `-n`, then read about 80 lines around it with offset and limit.
- **Never stream build or game output.** Send it to a log in your scratchpad, then search the log:
  `CoH\Scripts\install.ps1 *> $log; Select-String -Path $log -Pattern 'error|warning C' | Select -First 30`.
  A full build's output is tens of thousands of lines.
- **Search the retail data as files, not archives.** Run `python CoH/Tools/retail.py` once per checkout. It writes `build/retail/` (§5).
  Grep it (`^Object Raptor`, `^CommandSet AmericaDozer`) and read the block you need; never read a whole INI file into context.
- **Broad questions go to an Explore subagent** ("where does the engine decide X?"). It returns the answer, not the file dumps.
- **Write down what you learn as you go.** Put engine findings in your slice README's *Engine notes* section as soon as you find them.
  If the context is compacted, or the next session picks up, they aren't lost and don't need to be found again.
- **Prior art:** before working out a hook point yourself, check whether the old IbmEngine work already found it (§7).
  It is reference only, never copied in.

## 3. CoH itself

| Path | What |
|---|---|
| `CoH/Kernel/Kernel.h` | `COH_FEATURE(Name, init, reset)`, `registerIniBlock`, `COH_ADD_MODULE`, flags |
| `CoH/Kernel/Hooks.h` | the hub: one event per engine point; features `subscribe(self, fn)` |
| `CoH/Kernel/Time.h` | C++20 wrapper: `now`, `everySeconds`, `secondsFromNow`; pure `framesFromSeconds`, `isEvery` in `Domain/Frames.h` |
| `CoH/Kernel/Random.h` | C++20 wrapper: synced `chance(p)`, `pick(items)`, `between(lo, hi)` over `GameLogicRandomValue` |
| `CoH/Kernel/Objects.h` | C++20 wrapper: `objectsNear` (owns the partition iterator), `objectsOf(player)`, `allObjects`, an `ObjectFilter` with designated initializers, `kinds(...)`, `findObject`, `findLiving` |
| `CoH/Kernel/Players.h` | C++20 wrapper: `players`, `sides`, `activeSides`, `aiSides` views; `isEnemy`, `isAlly` |
| `CoH/Kernel/Ini.h` | C++20 wrapper: `Ini::Table` of `field` (parser from the member's type), `seconds` (kept as frames), `percent`, `custom`; for config blocks and module data |
| `CoH/Kernel/Domain/` | flags, the feature set, argument parsing, frames and chance rolls, as pure code |
| `CoH/Kernel/Engine/` | packs, game options sync, the save block, quick skirmish, `CoH.log` |
| `CoH/Features/Sectors/` | territory economy: capture, income, the cut-off graph, build rule, AI expansion, ground, radar and tooltip UI |
| `CoH/Features/GeneralsArsenal/` | command points, sortie and air cover powers, super weapons with a warning, AI power use; its README has *Engine notes* on special powers |
| `CoH/Features/Upkeep/` | population, its cap, upkeep on income, the AI's army limit |
| `CoH/Features/FireRate/` | slower, harder-hitting fast weapons through the `weaponBonus` hub point; a weapon's fire rate over its clip, reload and spin-up |
| `CoH/Features/Squads/` | infantry squads: made per order, one selection, formation moves, one unit in containers and population, the squad health bar; `Squads.h` for Reinforce, Retreat, Abilities, Weapon pickup |
| `CoH/Features/Pacing/` | unit prices and build times by class or unit, the skirmish AI's handicap by difficulty, the AI scripts' gifts of money |
| `CoH/Features/Accuracy/` | hit rolls for direct fire, misses beside the target; `Accuracy.h` has the hit modifier stack Cover, Suppression and Veterancy plug into |
| `CoH/Features/DirectionalArmor/` | front, side and rear armor, penetration by range, deflections; vehicles' guns against infantry; AI flanking; `DirectionalArmor.h` has `penetrationChanceFrom` for Vehicle criticals (08) |
| `CoH/Features/UnitScale/` | CoH proportions: each vehicle's `Scale` and `Geometry` by class, in a generated pack; `-cohUnitScaleShowcase` lines units up for screenshots |
| `CoH/Docs/CONTEXT.md` | this map |
| `CoH/Scripts/` | `install.ps1` (build and install), `package.ps1` (players' release zip; README *Release*), `skirmish.ps1` (test game), `replaycheck.ps1` (determinism); the shared build steps are in `Common.ps1` |
| `.github/workflows/coh-release.yml` | on a pushed `coh-v*` tag: build, `package.ps1 -NoBuild`, attach the zip to a GitHub release |
| `CoH/Kernel/Engine/Stats.cpp` | `-cohStats [seconds]`: each side's income, units built, army and its worth in `CoH.log`, with any flags (retail too); `-cohLog <name>` names the log for games side by side |
| `CoH/Tools/retail.py` | extracts the retail data to `build/retail/`; `read_big` and `read_csf` for packs that generate INI |

Worked examples in Sectors, to copy the shape of:
- a module with saved state that's part of the CRC: `Engine/SectorPointUpdate`
- an INI block: `Engine/Config`
- AI script rewriting: `Engine/AIScripts.cpp`
- AI build list control: `Engine/AIBuildList.cpp`
- ground and radar drawing: `UI/GroundOverlay`, `UI/RadarOverlay`; geometry made once and culled in chunks, lines hidden behind hills: `UI/BorderGeometry`; smooth lines and outlines as pure code: `Domain/BorderLines`
- a tooltip hook: `UI/MoneyTooltip`
- a generated pack: `Data/make_pack.py`

## 4. Engine map

`Z` = `GeneralsMD/Code/GameEngine/Source`, `C` = `Core/GameEngine/Source`, line counts in brackets.
Headers are under the matching `Include/`. Zero Hour code is in `GeneralsMD/`; `Generals/` is the original game and is never touched.

### Frame and messages

| Thing | Where |
|---|---|
| Game start, frame, end | `Z/GameLogic/System/GameLogic.cpp` [5.4k]: `tryStartNewGame` 1145, `update` 3691, `destroyObject` 4098, `reset` 408 |
| Player orders (move, attack, power, build) | `C/GameLogic/System/GameLogicDispatch.cpp` [2.4k], the `MSG_*` switch from about line 500 |
| Player selection | in game logic: `Player::m_currentSelection` (a `Squad` of IDs, no duplicate check); `GameLogic::selectObject`/`deselectObject` (`GameLogic.cpp` 2721; also called on death); hub `playerSelection` for the player's own messages; Squads `Engine/Orders.cpp` |
| Every unit command | hub `aiCommand` (`AIUpdateInterface::aiDoCommand`); infantry group moves arrive as `AICMD_FOLLOW_PATH`. Re-issue orders at frame end, not inside it (Squads `Engine/Orders.cpp`) |
| Replays | `Z/Common/Recorder.cpp` [1.8k]: `updatePlayback` 417 (appends a frame's recorded commands to `TheCommandList`), `updateRecord` 482; the frame 0 CRC is taken in `GameLogic::update` after the scripts, before the commands |
| Synced random | `Core/GameEngine/Include/GameLogic/LogicRandomValue.h`: `GameLogicRandomValue(lo, hi)`, `GameLogicRandomValueReal` |
| Objects in range | `ThePartitionManager->iterateObjectsInRange(pos, r, FROM_CENTER_2D, filters)`, `GeneralsMD/Code/GameEngine/Include/GameLogic/PartitionManager.h` 1413 |
| Save and load | `Xfer` and `Snapshot`; examples are `Player::xfer` (`Z/Common/RTS/Player.cpp` around 4050) and Sectors' `SectorPointUpdate`. State outside modules: `CoH::registerSaveData` (Kernel.h), loaded after the objects; example Squads `Engine/Save.cpp`. Blocks are listed in `GameState::init` (`Z/Common/System/SaveGame/GameState.cpp` 310); unknown blocks are skipped on load |

### Combat

| Thing | Where |
|---|---|
| Firing | `Z/GameLogic/Object/Weapon.cpp` [3.7k]: `Weapon::privateFireWeapon` 2621, `WeaponTemplate::fireWeaponTemplate` 774, projectiles inside it |
| Shot redirect (a miss) | hub `weaponFiringAtObject` in `fireWeaponTemplate`, after the range checks: a retargeted shot becomes a shot at a spot; Accuracy `Engine/Shots.cpp`. A hit modifier goes through `Accuracy.h` `addHitModifier`, not a new hook |
| Damage dealt | `WeaponTemplate::dealDamageInternal` 1377 (victims, radius, damage type), then `Object::attemptDamage` (`Object.cpp` 1906), then `ActiveBody::attemptDamage` (`Body/ActiveBody.cpp` 354). Hub `weaponDetonated` is where the damage goes off (before the victims are found; DirectionalArmor deals its shell blast there), `weaponDamage` changes one victim's amount before `attemptDamage`; `damageInfo.in.m_sourceID` is a projectile's launcher |
| **Damage order** (one shot, one victim) | 1. hit or miss: Accuracy, hub `weaponFiringAtObject`, at the fire. 2. cover: Cover, hub `weaponDamage`. 3. penetration and vehicles' guns against infantry: DirectionalArmor, hub `weaponDamage`, after Cover by feature order in `CoH/CMakeLists.txt`. 4. the engine's armor (`Armor.ini`) and damage scalar in `ActiveBody::attemptDamage`. 5. veterancy: Veterancy, hub `damageReceived`, after armor. Steps 2 to 5 multiply, so only the rolls depend on the order |
| Weapon bonuses, reload | `Weapon::computeBonus` 1913, `getDelayBetweenShots` 502, `getClipReloadTime` 520 and 1953; hub `weaponBonus` changes the summed bonuses (FireRate). Read delays with `getMin/MaxDelayBetweenShots` (`getDelayBetweenShots` draws synced random); a gattling's spin-up is `CONTINUOUS_FIRE_FAST` in `getExtraBonus()` |
| Weapon reach | `Weapon.cpp` `ATTACK_RANGE_CALC_TYPE` (86): ranges are measured between the units' bounding circles (`FROM_BOUNDINGSPHERE_2D`), so bigger geometry reaches further |
| Weapon choice | `Z/GameLogic/Object/WeaponSet.cpp` [1.2k] |
| Veterancy | `Z/GameLogic/Object/ExperienceTracker.cpp` [300]: `addExperiencePoints` 162, `setVeterancyLevel` 120; `Object::onVeterancyLevelChanged` 3169 |
| Kills and score | `Object::scoreTheKill` 2993, `Player::addSkillPointsForKill` 2528 |
| Disabled states | `Object::setDisabled` 2147, `setDisabledUntil` 2153 (stun, immobile, and so on) |

### Units and AI

| Thing | Where |
|---|---|
| Unit orders | `Z/GameLogic/Object/Update/AIUpdate.cpp` [5.4k]: `aiDoCommand` 2627, `privateMoveToPosition` 2894, `privateAttackObject` 3431, `privateEnter` 3815, `privateGuardPosition` 4112 |
| Speed | `AIUpdateInterface::chooseLocomotorSetExplicit` 834; `Z/GameLogic/Object/Locomotor.cpp` [2.8k] |
| AI state machine | `Z/GameLogic/AI/AIStates.cpp` [7.5k]: grep the state class, never read it whole |
| Group orders | `Z/GameLogic/AI/AIGroup.cpp` [3.4k] |
| Skirmish AI | `Z/GameLogic/AI/AISkirmishPlayer.cpp` [1.2k]: `processBaseBuilding` 94, `acquireEnemy` 480, `processTeamBuilding` 857; the shared parts are in `AIPlayer.cpp` [3.9k] |
| AI scripts | `build/retail/Scripts/SkirmishScripts.scb` (binary; §5); the engine side is `Z/GameLogic/ScriptEngine/` [ScriptEngine 10k, Actions 7.8k, Conditions 3k] |
| AI teams | `Z/Common/RTS/Team.cpp` [2.8k], `TeamTemplateInfo`; the retail teams are in the `.scb` |
| Pathfinding | `Core/GameEngine/Source/GameLogic/AI/AIPathfind.cpp` [7.5k]; avoid it |
| A unit's size | `Scale` (ThingTemplate) draws the model at Scale² (`W3DModelDraw::doDrawModule` scales a model already made scaled); the logic's bones match it. `Geometry*` is what logic sees: pathfinding footprint (`Pathfinder::getRadiusAndCenter`, 1 to 5 cells), weapon reach, collisions. UnitScale README, *Engine notes* |
| Containers and garrisons | `Z/GameLogic/Object/Contain/OpenContain.cpp` [2k], `TransportContain.cpp`, `GarrisonContain.cpp`; capacity through hub `containRoom` (`isValidContainerFor`: transports count slots, garrisons objects) |
| Unit made by a factory | hub `unitProduced` (`ProductionUpdate::update`, after the door and the team); more objects may leave with `getObjectExitInterface()->exitObjectViaDoor(obj, DOOR_1)`; `QuantityModifier` makes several per order (Red Guards). Squads `Engine/Production.cpp` |
| Spawned members | `Z/GameLogic/Object/Behavior/SpawnBehavior.cpp` [1.2k], `Update/MobMemberSlavedUpdate.cpp` |
| Can do X? | `Z/Common/RTS/ActionManager.cpp` [2.2k]: `canEnterObject` 537, `canCaptureBuilding` 1010, `canDoSpecialPowerAtLocation` 1482, `canDoSpecialPowerAtObject` 1640, `canDoSpecialPower` 1902 |

### Players, economy, powers

| Thing | Where |
|---|---|
| Money | `Z/Common/RTS/Money.cpp` [214]: `deposit` 82, `withdraw` 57 |
| Player | `Z/Common/RTS/Player.cpp` [4.6k]: `update` 669, `onUnitCreated` 1553, `canBuild` 2946 |
| Rank and science | `Player::addSkillPoints` 2503, `setRankLevel` 2720, `addSciencePurchasePoints` 2621, `attemptToPurchaseScience` 2635, `grantScience` 2658; `Z/Common/RTS/Science.cpp` |
| Special powers | `Z/GameLogic/Object/SpecialPower/SpecialPowerModule.cpp` [926]: `isReady` 311, `initiateIntentToDoSpecialPower` 462, `startPowerRecharge` 421, `doSpecialPowerAtLocation` 750; the templates are in `Z/Common/RTS/SpecialPower.cpp` |
| Power types | `GeneralsMD/Code/GameEngine/Include/Common/SpecialPowerType.h`; append only, so saved IDs don't change. A new type also goes in `ActionManager::canDoSpecialPowerAtLocation` (the cursor) |
| Power use and payment | hub `specialPowerUse` (veto: `SpecialPowerStore::canUseSpecialPower`, `AIGroup::groupDoSpecialPower*`, ControlBar greying) and `specialPowerTriggered` (`SpecialPowerModule::triggerSpecialPower`); GeneralsArsenal `Engine/Powers.cpp` |
| Command points (earning) | hub `killScored` (`Object::scoreTheKill`) and `damageDealt` (`ActiveBody::attemptDamage`); GeneralsArsenal `Engine/Earning.cpp` |
| Sorties | INI only: OCL `DeliverPayload` with a jet copy carrying `DeliverPayloadAIUpdate`; GeneralsArsenal `Data/make_pack.py` |
| Air cover | GeneralsArsenal `AirCoverZoneUpdate` (jets with a plain `AIUpdateInterface`; `JetAIUpdate` starts on the runway) |
| Production limits (population, max of a type) | hub `production` veto in `Player::canBuildMoreOfType`, which `BuildAssistant::canMakeUnit` (buttons, `ProductionUpdate::queueCreateUnit`), `Player::canBuild` and `isPossibleToMakeUnit` (the AI's `findFactory`) all ask; Upkeep `Engine/Rules.cpp` |
| Income | hub `income` in `Money::deposit` (tracked income only); Sectors pays with `deposit(amount, FALSE)`; Upkeep takes its share there |
| Super weapon warning | GeneralsArsenal `StrikeCountdownUpdate` (marker, then a hidden building copy at `TerrainLogic::findClosestEdgePoint` fires its power) and `UI/MarkerOverlay.cpp` |
| Cost and build time | `Core/GameEngine/Source/Common/Thing/ThingTemplate.cpp`: `calcCostToBuild` 1567, `calcTimeToBuild` 1586; hub `buildCost`, `buildTime` change them (Squads, then Pacing) |
| The skirmish AI's money cheats | `SkirmishScripts.scb` scripts "<Side> AI Bonus Early/Mid/Late/Really Late" (Hard, $55,000 by minute 13 on the `_ESCALATION` counter) and "Give AI money to cover bug"; `PLAYER_GIVE_MONEY` is a tracked deposit (`ScriptActions::doGiveMoney`). Pacing `Engine/AIScripts.cpp` scales them |
| Modules | `Z/Common/Thing/ModuleFactory.cpp`, `ModuleFactory::init` 321; features use `COH_ADD_MODULE` |
| INI blocks | `C/Common/INI/INI.cpp`, the block table; features use `CoH::registerIniBlock` |

### UI

| Thing | Where |
|---|---|
| Command bar | `C/GameClient/GUI/ControlBar/ControlBar.cpp` [3.8k], `ControlBarCommand.cpp` [1.6k] (`updateContextCommand` 715) |
| Passenger buttons and pips | a transport's: `ControlBar::doTransportInventoryUI`, `populateInvDataCallback` (`ControlBarCommand.cpp` 89); a garrison's: `ControlBarStructureInventory.cpp`; the pips: `Drawable::drawContained` (`Drawable.cpp` 2936). A click sends `MSG_EXIT` for the rider (`ControlBarCommandProcessing.cpp` 671). Hub `riderShown` hides riders another speaks for (Squads) |
| Tooltips | `C/GameClient/GUI/GUICallbacks/ControlBarPopupDescription.cpp`, `ControlBar::populateBuildTooltipLayout` 244 |
| General's power screen | `C/GameClient/GUI/GUICallbacks/GeneralsExpPoints.cpp`; `ControlBar::populatePurchaseScience` (`ControlBar.cpp` 178) |
| World overlay, floating text | `Z/GameClient/InGameUI.cpp` [6.4k]: `postDraw` 3785, `addFloatingText` 5366 |
| 2D drawing | `TheDisplay` (`Core/GameEngine/Include/GameClient/Display.h`): lines, rects, images, batched inside `W3DInGameUI::draw`. Gradient-filled quads through the hub's `drawGradientQuad` (`W3DDisplay::drawGradientQuad`). No depth test: Sectors' `UI/GroundOverlay` hides lines behind hills itself |
| Unit icons and bars | `Z/GameClient/Drawable.cpp` [5.7k]: grep `drawHealthBar`, `drawVeterancy`, `drawEmoticon`; hub `healthBar` changes the bar's values and may draw beside it (Squads `UI/HealthBar.cpp`) |
| Radar | `Core/GameEngine/Include/Common/Radar.h`, `Core/GameEngineDevice/Source/W3DDevice/Common/System/W3DRadar.cpp` |
| Voice warnings | `Z/GameClient/Eva.cpp`, `Eva::setShouldPlay` 372 |

## 5. Retail data (`build/retail/`, made by `python CoH/Tools/retail.py`)

| Path | What |
|---|---|
| `INI/Object/<Faction><Kind>.ini` | units and buildings: `AmericaAir`, `AmericaVehicle`, `ChinaInfantry`, `GLAVehicle`, and so on |
| `INI/Object/<General>General.ini` | each general's own versions: `AirforceGeneral`, `SuperWeaponGeneral`, `NukeGeneral`, `LaserGeneral`, `InfantryGeneral`, `TankGeneral`, `ChemicalGeneral`, `DemoGeneral`, `StealthGeneral` |
| `INI/Object/SpecialPowerObjects.ini` | strike aircraft and the objects the powers spawn |
| `INI/Weapon.ini`, `INI/Armor.ini`, `INI/Locomotor.ini` | weapons, armor tables, movement |
| `INI/CommandSet.ini`, `INI/CommandButton.ini` | build menus and buttons |
| `INI/SpecialPower.ini`, `INI/Science.ini`, `INI/Rank.ini`, `INI/Upgrade.ini` | powers, promotions, rank thresholds, upgrades |
| `INI/PlayerTemplate.ini` | factions and generals, starting units, purchasable sciences |
| `INI/AIData.ini` | skirmish AI tuning and its build lists |
| `INI/GameData.ini` | global settings |
| `strings.txt` | display text, as `LABEL = text` (`OBJECT:Raptor = Raptor`) |
| `Scripts/SkirmishScripts.scb` | the skirmish AI's scripts, binary: chunked, a `CkMp` TOC and then chunks. Sectors reads it at run time (`Engine/AIScripts.cpp`); decode it with a small scratch script rather than reading it raw |
| `files.txt` | every file in every `.big`, with its archive |

## 6. Build and test, in short

The full steps are in `CoH/README.md`, *Install* and *Testing*.

- Install: `CoH\Scripts\install.ps1 *> $log`, then check the log. It refuses while the game is running.
- Unit tests: configure once with `-DRTS_BUILD_OPTION_TESTS=ON`; build target `z_googletest --config Release`; run with `--gtest_filter=<Slice>*`.
- Test game, which the user watches: `CoH\Scripts\skirmish.ps1 -Flags X -Windowed`. AI against AI: `-cohSide Observer -cohOpponents 2`.
  Use `-cohLog` and grep `CoH.log` in the user data folder, rather than reading it.
- Determinism: `CoH\Scripts\replaycheck.ps1` (headless). Retail `GeneralsReplays` always mismatch on this compiler; that's expected.
- Replays run in `GAME_REPLAY` mode. A check like `isInSkirmishGame()` must also accept replays, or playback mismatches at frame 0.
- A frame 0 mismatch with every flag off, too, is the engine. To find one, log each part of `GameLogic::getCRC` (objects, partition, players, `TheAI`) at the frame 0 CRC while recording and while playing back, and diff. `TheAI` holds the AI group IDs, which any group made earlier shifts.

## 7. Prior art (reference only)

The old work is on branch `ibm-engine`. Read it with `git show ibm-engine:<path>`; never merge or copy it.
Its hooks table (`git show ibm-engine:IbmEngine/README.md`, *Hooks in original code*) lists engine points it already found and used.

| Task | Old slice | Hook points it used |
|---|---|---|
| 04 Accuracy | `IbmEngine/Features/Accuracy` | `WeaponTemplate::fireWeaponTemplate`: the roll before the fire effects; a miss lands near the target |
| 07 Directional armor | `IbmEngine/Features/Armor` | `WeaponTemplate::dealDamageInternal`: the penetration roll |
| 10 Reinforce | `IbmEngine/Features/Squads` (`SquadReinforce.cpp`) | refilling near the Barracks. Squads (09) is done differently (no leader object): build on `Squads.h` (`emptySlots`, `unitOfSlot`, `addMember`) |
| 02 Arsenal | `IbmEngine/Features/AirPower`, `WatchZones` | `canDoSpecialPowerAtLocation`, `DeliverPayloadAIUpdate` (jets following a target), new power types appended |
| 03b Pacing | `IbmEngine/Features/Economy` | `ThingTemplate::calcCostToBuild` and `calcTimeToBuild` (now the hub's `buildCost`, `buildTime`), `ActiveBody` constructor, `Weapon::computeBonus` |

## 8. Gotchas already paid for

- The skirmish AI scripts gate buildings on `BUILT_BY_PLAYER [SupplyCenter]`. Sectors renames those conditions at game start (`AIScriptRename`).
- A priority build-list entry the AI can't place blocks all its building.
- Packs that change the same command set add up only if each lists just the slots it changes. `N = None` empties a slot (a Kernel `// CoH fix` in `ControlBar`).
- `SpecialPowerStore` and similar stores must keep IDs stable across games, or replays of later games break.
- A multi-config build without `--config Release` builds Debug, and the installed exe stays old.
- A pack's new object lists all its modules inside `AddModule`; a `Locomotor` line goes after the AI module; a block names only what an earlier file defined. Crash reasons are in `ReleaseCrashInfo.txt` in the user data folder.
- `objectCreated` fires before a loaded object gets its saved ID: register modules (constructor and destructor), not object IDs. State kept by object ID is saved with `CoH::registerSaveData`, which loads after the objects.
- A skirmish can be saved: the Squads self-test saves with `TheGameState->saveGame` from its logic frame, and `-loadsave <file>` loads a save from the save folder at startup.
- `Object::crc` doesn't include modules, and `Player::computeSuperweaponTarget` draws synced random numbers (never call it just to log).
- Upgraded command sets (`CommandSetUpgrade`, such as China's airfield after the mines upgrade) need a pack's changes too.
- Several `-cohSkirmish` games can run at once (own `CoH_InstanceNN.log`); replacing the installed exe needs every game closed.
- A quick skirmish takes its starting money from the skirmish preferences (the user's is $50,000): pass `-cohCash 10000` for economy tests. `-win` without `-xres 1600 -yres 900` makes a window taller than the screen.
- A Hard AI's money is more than half script gifts once supplies are gone (Pacing README, *Measurements*): measure an economy change with `-cohStats` and count them in.
- `near` and `far` are empty macros from the Windows headers: a member named that fails with "syntax error: '='", even in Domain code.
- **Merging upstream** goes `upstream/main` → `main` → `coh/main`, never straight into `coh/main`. `main` has no commits of its own, so the first step is a fast-forward. Run `replaycheck.ps1` after each merge: the flags-off `Replays\CoH` and the flag-on `CoHSectorsFix`, `CoHArsenal`, `CoHAccuracy` replays (more in the other `CoH*` folders). Conflicts with a `// CoH hook` or `// CoH fix` keep upstream's change and re-apply ours; a hook that moved gets its row in the `CoH/README.md` hooks table changed. Before re-implementing a dropped fix, check `git log upstream/main -- <file>`: upstream may have fixed it. A replay that mismatches after a merge: play it on the pre-merge exe (another test folder) first. If it fails there too, the recording is stale (a later pack change, as Pacing retuned Sectors and Upkeep), not upstream: re-record it with the same flags and sides, and keep the old one in a `stale-<date>` folder.
- A pack can't change `Armor.ini`: `ArmorStore::parseArmorDefinition` clears and refills the template in place, with no override, so the change outlives the game (flag off would no longer be original). Change damage against an armor in code (DirectionalArmor's `InfantryDamage`).
- A pack's `Weapon` block for an existing weapon does nothing: `WeaponStore::newOverride` fills a copy it never stores, and objects keep the template their `WeaponSet` named at load. New weapon names do load. Change an existing weapon in code (hub `weaponBonus` for its bonuses, `weaponDetonated` and `weaponDamage` for its damage; DirectionalArmor's `Blast`).
- Smart App Control can block a freshly linked `generalszh.exe` or `z_googletest.exe` ("An Application Control policy has blocked this file"): touch a `.cpp` and relink for a new hash; the next one usually starts.
