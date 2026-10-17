# CoH: Company of Heroes ideas, C&C style

Modernize Zero Hour with Company of Heroes ideas while it stays a C&C game: fast, base building, three factions, General's powers.
The aim is fewer, more valuable units that survive, heal, gain veterancy and are worth their price, fighting over territory instead of mining it.

This is a fresh start from `main`. Earlier `IbmEngine/` work on other branches is reference only and is not carried over.

## Direction

- **Territory is the economy.** Supplies come from capturable sectors. There are no supply centers, supply trucks, supply docks or gathering workers.
  A sector is taken by conquest: forces stand on its point while no enemy contests it. No capture upgrade is needed.
- **The sky belongs to the General.** Fixed-wing aircraft and super weapons are General's powers, bought with General's points and called in.
  Helicopters stay part of the army and are built like other units.
- **CoH combat.** Hits can miss, cover matters, machine guns suppress, armor has a front and a rear, vehicles take critical damage.
- **CoH units.** Infantry fights in squads that reinforce, retreat and earn veterancy.

## Branches

- `coh/main` is the integration branch, made from `main`.
- Each task works on `coh/<feature>` from `coh/main` and merges back when its Done list is met.
- Upstream merges go `main` -> `coh/main`.

## Layout

```
CoH/
  CMakeLists.txt          one target, compiled into the Zero Hour engine (z_gameengine)
  README.md               feature list, install, and the table of every hook in original code
  PLAN.md                 this file
  Tasks/                  one task file per feature
  Kernel/                 shared by every feature: flags, INI packs, hook hub, sync, test support
  Features/<Name>/
    README.md             what it does, its flag, its INI settings, its hooks
    <Name>.h              the slice's public header; the only file other slices may include
    Domain/               rules as plain C++ with no engine headers: unit tested
    Engine/               adapters: hook subscribers, modules, AI glue; the only code touching engine types
    UI/                   in-game display (rings, icons, bars, tooltips)
    Data/INI/             the feature's INI pack, loaded when its flag is on
    Tests/                Google Test cases for Domain/, built into z_googletest
```

## Conventions

1. **Original code stays original.** Original files only get calls into the hook hub, `CoH::Hooks::onSomething(...)`, each marked `// CoH hook`.
   Hooks in `Core/` are wrapped in `#if RTS_ZEROHOUR`. A bug fixed in original code is marked `// CoH fix`.
   Every hook and fix is listed in the table in `CoH/README.md`.
2. **Few, general hook points.** A feature subscribes to the hub (`Kernel/Hooks.h`) rather than editing original code.
   When a feature needs a new hook point, it adds one general point to the hub (named after the engine event, not the feature) and lists it.
3. **Vertical slices.** Everything about a feature lives in its folder. Slices talk to each other only through public headers (`<Name>.h`) or hub events, never through another slice's internals.
4. **Off means original.** Every feature has a flag. With every flag off, the game plays exactly like `main`, replays included.
5. **Rules in Domain, glue in Engine.** Numbers and decisions (hit chance, capture progress, income, penetration) are pure functions in `Domain/` with unit tests.
   `Engine/` reads engine state, calls `Domain/`, and applies the result.
6. **Tuning in INI.** Each feature has its own INI block (`CoHSectors`, `CoHAccuracy`, ...) in its pack. No magic numbers in code.
7. **Deterministic logic.** Game logic uses only synced randomness (`GameLogicRandomValue`) and logic-frame time, so replays and multiplayer stay in sync.
   Client randomness is for visuals only.
8. **Clean code.** Small functions with one job, names that say what they mean, early returns, no globals outside the Kernel, comments that explain why.
   Engine-facing code follows the engine's style (`m_` members, CamelCase types).
9. **The skirmish AI keeps up.** A feature that changes how the game is played also makes the skirmish AI play by it, so every merge is playable against the AI.

## Done, for every task

- Domain unit tests pass (`ctest`).
- Profile build installed with `--config Release` and play-tested in a skirmish against the AI, with the flag on and off.
- With the flag off, a retail replay still plays without a mismatch.
- The feature README and the hooks table in `CoH/README.md` are up to date.
- Merged into `coh/main`.

## Roadmap

| # | Task | Phase | Needs |
|---|---|---|---|
| 00 | [Kernel](Tasks/00-Kernel.md) | Foundation | none |
| 01 | [Sectors](Tasks/01-Sectors.md) | Territory | 00 |
| 02 | [General's air and super weapons](Tasks/02-GeneralsArsenal.md) | Territory | 00 |
| 03 | [Upkeep and population](Tasks/03-Upkeep.md) | Territory | 01 |
| 04 | [Accuracy](Tasks/04-Accuracy.md) | Combat | 00 |
| 05 | [Cover](Tasks/05-Cover.md) | Combat | 04 |
| 06 | [Suppression](Tasks/06-Suppression.md) | Combat | 04 |
| 07 | [Directional armor](Tasks/07-DirectionalArmor.md) | Combat | 00 |
| 08 | [Vehicle criticals](Tasks/08-VehicleCriticals.md) | Combat | 07 |
| 09 | [Squads](Tasks/09-Squads.md) | Units | 00 |
| 10 | [Reinforce](Tasks/10-Reinforce.md) | Units | 09, 01 |
| 11 | [Retreat](Tasks/11-Retreat.md) | Units | 09 |
| 12 | [Veterancy](Tasks/12-Veterancy.md) | Units | 00 |
| 13 | [Abilities](Tasks/13-Abilities.md) | Units | 09 |
| 14 | [Field defenses](Tasks/14-FieldDefenses.md) | Map | 05 |
| 15 | [Victory points](Tasks/15-VictoryPoints.md) | Map | 01 |
| 16 | [Doctrines](Tasks/16-Doctrines.md) | Commanders | 02 |
| 17 | [Weapon pickup](Tasks/17-WeaponPickup.md) | Stretch | 09 |

Tasks whose needs are met can run at the same time on their own branches.
After the Kernel, 01, 02, 04, 07, 09 and 12 can all start.
