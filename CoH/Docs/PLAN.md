# CoH: Company of Heroes ideas, C&C style

Modernize Zero Hour with Company of Heroes ideas while it stays a C&C game: fast, base building, three factions, General's powers.
The aim is fewer, more valuable units that survive, heal, gain veterancy and are worth their price, fighting over territory instead of mining it.

This is a fresh start from `main`. Earlier `IbmEngine/` work on other branches is reference only and is not carried over.

## Direction

- **Territory is the economy.** Supplies come from capturable sectors. There are no supply centers, supply trucks, supply docks or gathering workers.
  A sector is taken by conquest: forces stand on its point while no enemy contests it. No capture upgrade is needed.
- **The sky belongs to the General.** Fixed-wing aircraft and super weapons are General's powers, called in and paid per use with command points earned in combat.
  Helicopters stay part of the army and are built like other units.
- **CoH combat.** Hits can miss, cover matters, machine guns suppress, armor has a front and a rear, vehicles take critical damage.
- **CoH units.** Infantry fights in squads that reinforce, retreat and earn veterancy.

## Branches

- `coh/main` is the integration branch, made from `main`.
- Each task works on `coh/<feature>` from `coh/main` and merges back when its Done list is met.
- Upstream merges go `main` -> `coh/main`.

## Layout

`CoH/` is made of main packages, one folder each. Vertical slices live only inside `Features/`.

```
CoH/
  README.md               entry point: feature list, install, testing, the hooks table
  CMakeLists.txt          one target, compiled into the Zero Hour engine (z_gameengine)
  Kernel/                 framework shared by every feature: registry, flags, INI packs, hook hub, sync
  Features/               one vertical slice per feature:
    <Name>/
      README.md           what it does, its flag, its INI settings, its hooks, engine notes
      <Name>.h            the slice's public header; the only file other slices may include
      Domain/             rules as plain C++ with no engine headers: unit tested
      Engine/             adapters: hook subscribers, modules, AI glue; the only code touching engine types
      UI/                 in-game display (rings, icons, bars, tooltips)
      Data/INI/           the feature's INI pack, loaded when its flag is on
      Tests/              Google Test cases for Domain/, built into z_googletest
  Scripts/                PowerShell for building, installing and testing (install, skirmish, replaycheck)
  Tools/                  developer tools (retail.py: retail data to build/retail/)
  Docs/                   PLAN.md (this file), CONTEXT.md (the context map), Tasks/ (one file per feature)
```

A new main package is added only for a new kind of thing, never for a feature.

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
10. **Modern C++20 for new code.** New CoH code is written in C++20 against the Kernel wrapper (`Kernel/Time.h`, `Random.h`, `Objects.h`, `Players.h`, `Ini.h`; see [K1](Tasks/K1-ModernCpp.md)).
    Use what makes it shorter and safer: `std::span`, `std::optional`, ranges and views, structured bindings, designated initializers, `constexpr`, `enum class`, lambdas, and concepts where they help.
    - `// CoH hook` calls inside original files stay in the original style.
    - Existing slices aren't converted. A slice keeps the style it was written in.
    - Avoid what breaks determinism: unordered containers (`std::unordered_map`, `std::unordered_set`) iterated in game logic, pointer-keyed ordering, `<random>` and `rand`, the clock (`<chrono>`) for game time, and floating-point library calls whose results may differ between compilers, CPUs or library versions (`std::sin`, `std::cos`, `std::pow`, `std::exp`, `std::log`, the C++17 special math functions) in game logic.
      Use the wrapper's `chance`, `pick` and frames, the engine's own math (`Coord3D`, `WWMath`), and plain float arithmetic.

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
| K1 | [Modern C++: the C++20 wrapper](Tasks/K1-ModernCpp.md) | Foundation | 00; the next feature task is written against it |
| 01 | [Sectors](Tasks/01-Sectors.md) | Territory | 00 |
| 02 | [General's air and super weapons](Tasks/02-GeneralsArsenal.md) | Territory | 00 |
| 03 | [Upkeep and population](Tasks/03-Upkeep.md) | Territory | 01 |
| 03b | [Pacing: unit costs, build times, the AI's handicap](Tasks/03b-Pacing.md) | Territory | 01, 03 |
| 03c | [Resources: manpower, fuel, munitions; command points unlock tech](Tasks/03c-Resources.md) | Territory | 01, 02, 03, 03b |
| 04 | [Accuracy](Tasks/04-Accuracy.md) | Combat | 00 |
| 04b | [Fire rate](Tasks/04b-FireRate.md) | Combat | 00 (pairs with 04) |
| 05 | ~~[Cover](Tasks/05-Cover.md)~~ cancelled 2026-10-19 | Combat | 04 |
| 06 | [Suppression](Tasks/06-Suppression.md) (postponed 2026-10-19) | Combat | 04 |
| 07 | [Directional armor](Tasks/07-DirectionalArmor.md) | Combat | 00 |
| 08 | [Vehicle criticals](Tasks/08-VehicleCriticals.md) | Combat | 07 |
| 09 | [Squads](Tasks/09-Squads.md) | Units | 00 |
| 10 | [Reinforce](Tasks/10-Reinforce.md) | Units | 09, 01 |
| 11 | [Retreat](Tasks/11-Retreat.md) | Units | 09 |
| 12 | [Veterancy](Tasks/12-Veterancy.md) | Units | 00 |
| 13 | [Abilities](Tasks/13-Abilities.md) | Units | 09 |
| 14 | [Trenches](Tasks/14-FieldDefenses.md) (later) | Map | 09 |
| 15 | [Victory points](Tasks/15-VictoryPoints.md) | Map | 01 |
| 16 | [Doctrines](Tasks/16-Doctrines.md) | Commanders | 02 |
| 17 | [Weapon pickup](Tasks/17-WeaponPickup.md) | Stretch | 09 |
| 19 | [Unit scale: CoH proportions](Tasks/19-UnitScale.md) | Units | 00 |
| 20 | [Vehicle movement: weight, then reversing](Tasks/20-VehicleMovement.md) | Units | 00, K1 |

Tasks whose needs are met can run at the same time on their own branches.
After the Kernel, 01, 02, 04, 07, 09 and 12 can all start.

### Spikes

Spikes try a new way of working, on real tasks. The user decides afterwards what to keep.

| # | Spike | Starts | Question |
|---|---|---|---|
| S1 | [A modern language for CoH code](Tasks/S1-LanguageSpike.md) | set aside: the user chose plain C++20 with the Kernel wrapper ([K1](Tasks/K1-ModernCpp.md)) | Do cppfront, Nim, Rust (`cxx`) or Lua beat plain C++20 for CoH code, without costing determinism or debugging? |
