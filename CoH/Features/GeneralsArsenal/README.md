# GeneralsArsenal

**Flag** `GeneralsArsenal` · **Task** [02](../../Tasks/02-GeneralsArsenal.md) · **Public header** [GeneralsArsenal.h](GeneralsArsenal.h)

The sky belongs to the General. Fixed-wing aircraft and super weapons are no longer built: they are General's powers, called in from off the map and paid for each time with **command points (CP)** earned in combat. Helicopters stay part of the army and are built at the Airfield.

## How it plays

### Command points

- Every player earns CP all the time from combat: a share of the experience value of the enemies their units damage and destroy. Damage earns by the share of the victim's health taken (`PointsPerDamageExperience`), the kill earns the rest (`PointsPerKillExperience`). A tank worth 100 experience gives about 5 CP over the damage and 10 for the kill. Fractions add up; CP stop at `MaxPoints`.
- CP are shown next to the money ("$ 2400   CP 85"); the money tooltip says how they are earned.
- **Every** General's power costs CP each time it is used, the old ones too (A-10, paradrop, carpet bomb, artillery, ...). Nothing is unlocked for good by buying it.
- Rank still comes from experience and opens the powers in tiers, at ranks 1, 3 and 5. A power's levels (A-10 strike 2 and 3, paradrop 2 and 3, ...) come with the ranks after its tier. The powers' sciences are no longer on the promotion screen; passive promotions (Paladin, Advanced Training, ...) are still bought there with promotion points.
- Each power's button and shortcut is greyed while it can't be used, and its tooltip says why: locked by rank, needs an Airfield, cooling down (the clock), or not enough CP. It also shows the cost, rank, cooldown and the loss penalty.

### Air powers

- No jet or bomber can be built: Raptor, King Raptor, Stealth Fighter, Aurora (and Aurora Alpha), MiG and every general's variant are gone from the airfields.
- **Sorties** (strike powers): the planes fly in from the map edge nearest the base, fire the weapon of the jet they replace at the target area and leave. They look and hit like that jet.
- **Air cover** (patrol powers): jets come in from the nearest map edge and patrol the area for 45 seconds, attacking what enters: an air superiority patrol attacks aircraft (called-in planes too), a ground attack patrol attacks vehicles. Then they fly off the map. Duration, radius, count and targets are set per power.
- **Losses:** anti-air shoots called-in planes down as it always did. Each plane lost makes the power that sent it ready later (its `LossPenalty`; all losses together at most `MaxLossDelay` past a full cooldown), so air defense and escorts matter. A plane that leaves the map isn't lost.
- **The Airfield** builds only helicopters (and its upgrades), and while one stands it is required for that player's air powers, the old ones included (A-10, Spectre, carpet bomb, daisy cutter, paradrop, napalm strike, ...). GLA has no airfield, so its old powers keep working as they did.

### Super weapons

- The Particle Cannon, Nuclear Missile and Scud Storm buildings can't be built. Each is a rank-5 power with a high CP cost and a long cooldown, on the command center.
- **The warning:** after the target is picked, every player sees it marked on the ground and on the minimap for 20 seconds, through the shroud, with the weapon's name and a countdown, and hears the usual super weapon voice warning. Then it fires, so units can scatter. This is the counterplay that destroying the building used to be.
- It looks and hits like the building did: a hidden copy of the building is made at the map edge as the countdown starts (so it can charge) and fires its own weapon at the target: the particle beam, the nuclear missile, the Scud Storm.
- The silo's upgrades (uranium shells, nuclear tanks, neutron shells) move to the general's Propaganda Center.

### Generals keep their identity

- **Granger (Air Force):** every air power costs a quarter less, his sorties send three planes instead of two, his air cover three jets, and he has an extra ground attack patrol (Stealth Fighters).
- **Alexander (Super Weapon):** the cheapest super weapon (180 CP) with the shortest cooldown (4 min).
- **Tao (Nuke):** a stronger nuke (blasts hit 1.5 times harder and 1.25 times wider), and his MiG strike fires tactical nukes.
- **The others:** each general's own planes and buildings are copied, so their variants (laser Raptors, Aurora Alpha's fuel air bomb, Black Napalm MiGs, toxin or demolition Scud Storms, ...) carry over; their old powers keep their own strength and cost CP by tier.

### GLA

GLA gets no air. In place of air powers it has ground powers, and the Scud Storm as its super weapon:
- **Hunter Ambush** (rank 1): four RPG troopers rise from hiding at the target. They hunt vehicles and shoot at aircraft, so it doubles as GLA's air cover.
- **Scud Barrage** (rank 3): three Scud missiles fall on the target a few seconds after it is picked (only the GLA player and allies see that warning).
- **Scud Storm** (rank 5): the super weapon, with the warning everyone sees.
- The old Rebel Ambush, Anthrax Bomb, GPS Scrambler, Sneak Attack, Cash Bounty and so on stay, paid with CP.

### Helicopters

- The Comanche, the Helix and the Chinook are built at the Airfield (the general's own). The Chinook moved there because Sectors (task 01) removed the Supply Center that built it; its prerequisite is the Airfield.

### Skirmish AI

- It earns CP like a player. Its own scripts keep using the old powers they know, and they now pay CP for them (the veto keeps a script from firing what it can't afford).
- Every 8 seconds (after the first 2 minutes) each AI may use one new power: a super weapon at the richest spot of an enemy base; a strike at the richest group of enemy units, or a base; air cover over its own units that are fighting what the cover is for. Once its rank allows a super weapon, it keeps 30% of its cost back from smaller powers.
- Super weapon buildings come out of its build lists; it can't train jets, so its jet teams are never picked.

## Powers

Cost in CP, cooldown in seconds, rank. "Air" powers need an Airfield; "loss" is the cooldown added per plane shot down.

| Power | Generals | Cost | Cooldown | Rank | Air | Loss |
|---|---|---|---|---|---|---|
| Raptor Strike (sortie, Raptors' missiles) | USA, Laser, Super Weapon | 40 | 120 | 1 | yes | 20 |
| Raptor Strike (King Raptors, 3 planes) | Air Force | 30 | 120 | 1 | yes | 20 |
| Stealth Strike (sortie, Stealth Fighters' bombs) | USA, Laser, Super Weapon / Air Force | 60 / 45 | 150 | 3 | yes | 25 |
| Aurora Strike (sortie, Aurora bombs; Aurora Alpha for Super Weapon) | USA, Laser, Super Weapon / Air Force | 80 / 60 | 180 | 3 | yes | 30 |
| Raptor Air Cover (air superiority, 45 s) | USA, Laser, Super Weapon / Air Force (3 jets) | 50 / 38 | 150 | 1 | yes | 15 |
| Stealth Ground Cover (ground attack, 45 s) | Air Force | 45 | 180 | 3 | yes | 20 |
| Particle Cannon | USA, Laser, Air Force / Super Weapon | 250 / 180 | 360 / 240 | 5 | no | |
| MiG Strike (sortie, napalm missiles; tactical nukes for Nuke) | China, Tank, Infantry, Nuke | 40 | 120 | 1 | yes | 20 |
| MiG Air Cover (air superiority, 45 s) | China, Tank, Infantry, Nuke | 50 | 150 | 3 | yes | 15 |
| Nuclear Missile | China, Tank, Infantry / Nuke (stronger) | 250 / 280 | 360 | 5 | no | |
| Hunter Ambush (RPG troopers) | GLA, Toxin, Demolition, Stealth | 40 | 180 | 1 | no | |
| Scud Barrage (3 Scuds) | GLA, Toxin, Demolition, Stealth | 70 | 180 | 3 | no | |
| Scud Storm | GLA, Toxin, Demolition, Stealth | 250 | 360 | 5 | no | |
| Every old General's power (A-10, Spectre, paradrop, carpet bomb, artillery, EMP, cash hack, ambush, ...) | its generals | 30 / 60 / 100 by rank 1 / 3 / 5; air ones of the Air Force general a quarter less | as before | its science's rank | air ones (USA, China) | 15 |

## INI

The pack is `Data/INI`:

| File | What |
|---|---|
| `10_GeneralsArsenal.ini` | the `CoHGeneralsArsenal` block (earning, losses, AI) and the ledger object |
| `20_Powers.ini` | generated: the new powers' `SpecialPower` blocks, a `CoHPower` block for every General's power, the power sciences off the promotion screen |
| `30_Weapons.ini` | generated: air cover copies of the jets' weapons that reload in the air |
| `40_Objects.ini` | generated: sortie and air cover jets, super weapon hosts, targets, air cover zones, Tao's nuke |
| `50_OCL.ini` | generated: what each new power creates |
| `60_Buttons.ini` | generated: the new powers' buttons and shortcuts |
| `70_Buildings.ini` | generated: the power modules on airfields and command centers, the loss rule on the old powers' planes, the Chinook at the Airfield |
| `80_CommandSets.ini` | generated: airfields, dozers and workers, command centers, the shortcut bars, the Propaganda Centers (one slot per line, so it adds up with Sectors' changes to the same sets) |
| `90_Text.ini` | generated: the new buttons' names and tooltips (`CoHText`) |

The generated files come from the retail `INIZH.big`; the power table above is the `POWERS` and `GENERAL_STYLE` tables of the generator: `python CoH/Features/GeneralsArsenal/Data/make_pack.py ["<Zero Hour folder>"]`.

`CoHGeneralsArsenal` (defaults in `Engine/Config.cpp`, pack values in `10_GeneralsArsenal.ini`):

| Setting | Pack | Meaning |
|---|---|---|
| `PointsPerKillExperience` | 0.1 | CP per experience point of an enemy killed |
| `PointsPerDamageExperience` | 0.05 | CP per experience point, times the share of its health taken |
| `MaxPoints` | 1000 | CP stop growing here |
| `MaxLossDelay` | 300 | s: losses never push a power more than this past a full cooldown started now |
| `AirfieldKinds` | FS_AIRFIELD | a finished, standing one allows the air powers |
| `AIRemoveFromBuildList` | FS_SUPERWEAPON | taken out of AI build lists |
| `AIThinkInterval` / `AIFirstUse` | 8 / 120 | s between AI picks; game time before the first |
| `AISuperweaponSaving` | 0.3 | share of an unlocked super weapon's cost the AI keeps back |
| `AIStrikeMinValue` / `AIStrikeRadius` | 1200 / 150 | build cost of enemy units a strike must hit, within this radius |
| `AICoverRadius` | 350 | enemies this close to the AI's units call for air cover |
| `LedgerObject` | CoHCommandLedger | the hidden object that keeps the CP |

`CoHPower <SpecialPowerTemplate>`, one per General's power:

| Setting | Meaning |
|---|---|
| `Cost` | CP per use |
| `MinRank` | the rank that opens it |
| `NeedsAirfield` | an Airfield must stand |
| `LossPenalty` | s added to its cooldown per plane of it shot down |
| `AIUse` | NONE (its scripts use it), STRIKE, COVER or SUPERWEAPON |
| `AITargetKinds` | for COVER: enemies of these kinds near its units call for it |
| `Description` | label of a line added to its tooltip |
| `Level = <science> <rank>` | its levels: the first is granted at the start, each other one at its rank |

The cooldown is the power's own `ReloadTime`. Modules the pack uses: `CommandPointsLedger` (the ledger), `SortieLossDie` (on called-in planes: the loss penalty), `AirCoverZoneUpdate` (a patrol), `StrikeCountdownUpdate` (a super weapon target and its host). Their settings are in their headers.

## Code

- `Domain/`: the rules, unit tested in `Tests/`. `CommandPoints` (earning, the wallet), `PowerRules` (rank gates, airfield, cost, the button state, levels by rank), `Cooldown` (loss penalties), `Warning` (the countdown), `AiChoice` (which power the AI uses).
- `Engine/`: `GeneralsArsenal.cpp` (declaration and hook subscribers), `Config` (the INI blocks), `Ledger` and `CommandPointsLedger` (CP, saved with the game), `Earning.cpp`, `Powers.cpp` (the veto, paying, science grants, loss penalties), the modules `SortieLossDie`, `AirCoverZoneUpdate` and `StrikeCountdownUpdate`, `Markers.cpp` (targets and zones in play), `AI.cpp`, `AIBuildList.cpp`, `Public.cpp` (`GeneralsArsenal.h`).
- `UI/`: `PointsAndTooltips` (CP next to the money, the money and button tooltips), `MarkerOverlay` (targets and zones on the ground and the minimap).
- `Data/make_pack.py`: the pack generator.

## Hooks

GeneralsArsenal subscribes to `gameStartAfterMap`, `logicFrame`, `objectCreated`, `objectDestroyed`, `gameEnd`, `inGameUIPostDraw`, `radarDraw`, `moneyTooltip`, and to these points it added to the hub (listed in the [hooks table](../../README.md#hooks-in-original-code)):

| Hub point | Original code | Used for |
|---|---|---|
| `killScored(killer, victim)` | `Object::scoreTheKill` | CP for kills |
| `damageDealt(victim, damage)` | `ActiveBody::attemptDamage` | CP for damage |
| `specialPowerUse(source, power)` (a `Veto`) | `SpecialPowerStore::canUseSpecialPower`, `ControlBar::getCommandAvailability` | rank, airfield and CP; the button greyed |
| `specialPowerTriggered(source, power, target)` | `SpecialPowerModule::triggerSpecialPower` | paying; telling planes which power sent them |
| `commandTooltip(button, text)` | `ControlBar::populateBuildTooltipLayout` | a power's cost, rank and state |
| `moneyDisplay(player, text)` | `InGameUI::update` | CP next to the money |

It also uses the Kernel's `CoHText` blocks (text a pack adds) and five special power types added at the end of the enum (`SPECIAL_COH_STRIKE_1..3`, `SPECIAL_COH_COVER_1..2`, so each new power has its own shortcut slot; the super weapons reuse the old types, which keep their voice warnings).

## Engine notes

What the slice found in the engine, for whoever works on powers next.

- **Powers' sources.** General's powers are `SpecialPowerModule`s on the command center (and here the airfield); `SharedSyncedTimer = Yes` keeps one cooldown per player in `Player::m_specialPowerReadyTimerList` (`getOrStartSpecialPowerReadyFrame`, `expressSpecialPowerReadyFrame`), keyed by template ID.
- **Three ways a power is fired**, all of which the veto must cover: a player's order (`AIGroup::groupDoSpecialPower*`, it skips `canUseSpecialPower`), `Object::doSpecialPower*` (asks `SpecialPowerStore::canUseSpecialPower`), and AI scripts (`ScriptActions::doSkirmishFireSpecialPowerAtMostCost` calls the module directly; its condition `evaluateSkirmishSpecialPowerIsReady` asks `canUseSpecialPower`). Every path ends in `SpecialPowerModule::triggerSpecialPower`, the one place to pay.
- **The targeting cursor** is `ActionManager::canDoSpecialPowerAtLocation`, a switch over `SpecialPowerType` with no default: a new type must be listed there, or players get the "can't" cursor while the AI (which skips it) works.
- **New special power types** go at the end of `SpecialPowerType` and `s_bitNameList`. The shortcut bar finds a power by type (`findMostReadyShortcutSpecialPowerOfType`), so two powers a player has at once need different types; it shows only the first `SpecialPowerShortcutButtonCount` slots of its set.
- **Special power IDs** were never wound back after a game (`SpecialPowerStore::reset`): pack-added powers drifted. Fixed (CoH fix), as `ThingFactory::reset` does for templates.
- **Sorties** need no code: an `ObjectCreationList` `DeliverPayload` whose `Transport` has `DeliverPayloadAIUpdate`, firing the jet's weapon through `VisiblePayloadTemplateName`/`VisiblePayloadWeaponTemplate` (the projectile object and weapon). The OCL marks transports script-targetable, so anti-air shoots them.
- **Jets can't just be spawned in the air**: `JetAIUpdate::onObjectCreated` sets the taxiing locomotor. Air cover jets use a plain `AIUpdateInterface` with the jet's `SET_NORMAL` locomotor and weapons copied with `AutoReloadsClip = Yes`.
- **Super weapons without a building**: a hidden copy of the building (its draw, body, power and update modules; KindOf `INERT` so it never counts as a unit) made at `TerrainLogic::findClosestEdgePoint` (the map's full extent, outside the playable area) fires its own power with `COMMAND_FIRED_BY_SCRIPT` (the particle cannon's automatic sweep). `MissileLauncherBuildingUpdate` pops its doors open once the power is ready. EVA warnings come from the power's `SpecialPowerType` at trigger (`SpecialPowerModule::aboutToDoSpecialPower`), so the new powers reuse the old types; the hosts' powers have none, so they don't warn twice.
- **Module registries**: `objectCreated` fires before a loaded object gets its saved ID, so keep module pointers registered in the module's constructor and destructor, not object IDs.
- **`Object::crc` doesn't include modules**: a module's `crc()` isn't part of the game's CRC check.
- **AI targets**: `Player::computeSuperweaponTarget` (the richest spot of an enemy's structures) draws `GameLogicRandomValue`, so calling it only for a log changes the game.
- **The AI's build lists** (`Player::getBuildList`) can be edited on the fly; teams with units no factory can build are never picked (`AIPlayer::isPossibleToBuildTeam` asks `BuildAssistant::isPossibleToMakeUnit`, which checks command sets).
- **Pack loading**: a new object's modules go inside `AddModule`, a `Locomotor` line after the AI module, and anything a block names (OCL, weapon, special power) must be defined in an earlier file. Upgraded command sets (`CommandSetUpgrade`, such as China's airfield after the mines upgrade) need the same changes as the base set.

## Testing

- `-cohUnlock` (for trying the powers): every human player starts at rank 5 with $100,000, and their command points stay full. Test games only; a replay of such a game needs the same option.

- Unit tests: `z_googletest.exe --gtest_filter=GeneralsArsenal*`.
- AI-vs-AI game to watch, from the game folder (`-cohOpponentSides` picks the generals; several can run at once, each with its own `CoH_InstanceNN.log`):
  `generalszh.exe -useCwd -win -coh GeneralsArsenal -cohSkirmish "Tournament Desert" -cohSide Observer -cohOpponents 2 -cohOpponentSides FactionAmericaAirForceGeneral,FactionChinaNukeGeneral -cohAI hard -cohLog -cohQuitAfter 900`.
  `CoH.log` in the user data folder lists every power used and what it cost, planes lost and the cooldown they added, super weapon hosts firing, air cover patrols, the sciences granted by rank, and once a minute each player's CP, rank and airfield.
