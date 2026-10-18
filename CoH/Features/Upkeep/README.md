# Upkeep

**Flag** `Upkeep` · **Task** [03](../../Docs/Tasks/03-Upkeep.md) · **Needs** [Sectors](../Sectors/README.md) · **Public header** [Upkeep.h](Upkeep.h)

A big army costs money to keep. Fewer, better units pay off; spamming units doesn't.

## How it plays

- **Population.** Each unit takes population by its build cost: one per `BuildCostPerPopulation` (250), rounded, at least `MinUnitPopulation`. A Ranger takes 1, a Crusader 4, an Overlord 8. Units that cost nothing (called-in planes, spawned helpers) and drones take none, and in the pack neither do dozers and workers. Units in production queues count too, so the queue can't overshoot the cap. With [Squads](../Squads/README.md) a squad counts once, by its whole price (a Ranger squad, 810, takes 3), through `Squads.h`.
- **The cap** starts low and rises with buildings and with sectors held: `CapBase` (6), plus each finished building of a `CapBuilding` kind (command center 8, barracks 2, war factory 3, airfield 2), plus `CapPerSector` (2) for each sector held, up to `CapMax` (60). A start with a command center and the home sector has 16; a base with a barracks, two war factories and five sectors 32.
- A unit that doesn't fit under the cap can't be made: its button is greyed and its tooltip says why. Workers and dozers (`NeverRefusedKinds`) take population but are always made, so nobody loses their builders. Units that come some other way (paradrops, ambushes) still count, even past the cap.
- **Upkeep** takes a share of every income, growing with population in steps (`UpkeepStep`): nothing below 20, 10% from 20, 25% from 35, 40% from 50. With Sectors almost all income is sector income; oil derricks, hackers and the like pay upkeep too. Starting money, selling and refunds don't.
- **On screen:** the population is next to the money ("$ 2400   Pop 34/60 (-10%)"). The money tooltip shows the income per minute as gross, upkeep and net, the population and cap, and the next step. A unit's build tooltip shows the population it takes.

## Skirmish AI

The AI plays under the same cap, and grows its army only while upkeep stays at or below `AIMaxUpkeepPercent` (10%): it stops before the 25% step, at 34 population, so most of its income stays free for rebuilding and its economy never stalls. A unit it can't make makes its factory search fail, so it picks other teams; teams it couldn't fill expire as they always did.

## Settings (`CoHUpkeep` in [Data/INI/10_Upkeep.ini](Data/INI/10_Upkeep.ini))

| Setting | Default | Meaning |
|---|---|---|
| `BuildCostPerPopulation` | 250 | a unit takes one population per this much of its build cost, rounded |
| `MinUnitPopulation` | 1 | a unit that costs money takes at least this |
| `Population = <object> <n>` | none | one unit's population, instead of by its cost |
| `UnitKinds` | INFANTRY VEHICLE AIRCRAFT | objects of any of these kinds take population... |
| `NonUnitKinds` | STRUCTURE DRONE | ...unless they are any of these |
| `NeverRefusedKinds` | DOZER HARVESTER | take population but are made even at the cap |
| `CapBase` | 10 (pack 6) | every player's cap starts here |
| `CapPerSector` | 3 (pack 2) | each sector held adds this |
| `CapMax` | 150 (pack 60) | the cap never goes higher |
| `CapBuilding = <KINDOF> <n>` | see the pack | each finished building of the kind adds n |
| `UpkeepStep = <population> <percent>` | see the pack | from that population on, that share of income is upkeep |
| `AIMaxUpkeepPercent` | 25 (pack 10) | the AI grows its army only while upkeep stays at or below this |

The pack's `Population`, `CapBuilding` and `UpkeepStep` lines replace the defaults rather than adding to them.

### The pack's numbers

Retuned in task [03b Pacing](../../Docs/Tasks/03b-Pacing.md); the measurements are in the [Pacing README](../Pacing/README.md#measurements). Before, a Hard AI reached its cap (about 50) by minute 3 and held 50 to 80 population from minute 5 on; the aim is about half, so each unit matters.

- Population stays one per $250 of retail cost: Pacing's higher prices don't raise it, so a unit costs more without taking more room.
- `CapBase` 6, `CapPerSector` 2, `CapBuilding` command center 8, barracks 2, war factory 3, airfield 2 (were 10, 3, 10, 4, 6, 4): buildings gave 26 to 30 of a 50 cap, so a big base alone filled it. Now a base with a barracks, two war factories and five sectors has 32, and holding ground matters as much as building.
- `CapMax` 60 (was 150): a side holding most of the map with a big base stops around 60, about a CoH army of fifteen squads and vehicles.
- `UpkeepStep` 20 10, 35 25, 50 40 (were 30, 60, 100): the steps follow the smaller cap, so a full army of 32 already pays 10%, and pushing past 35 costs a quarter of every income.
- `NonUnitKinds` adds DOZER: a $1,000 dozer took 4 of a cap of 32, and an AI with three or four of them had half its cap in builders.
- `AIMaxUpkeepPercent` 10 (was 25): the AI stops at 34, before the 25% step. With the old 25 it grew to 99 whenever its cap allowed it.

## Hooks

| Hub point | Used for |
|---|---|
| `production` (veto, `Player::canBuildMoreOfType`) | the cap: greys the button, refuses the queue order, and makes the AI's factory search fail |
| `income` (`Money::deposit`) | takes the upkeep share of each income before it is deposited and counted |
| `moneyDisplay`, `moneyTooltip`, `commandTooltip` | population next to the money, the income split, a unit's population |
| `gameStartAfterMap`, `logicFrame`, `gameEnd` | the game counts; once a minute the standing goes to the CoH log |

## Engine notes

- Population, cap and upkeep are counted from the player's objects each time they are asked (`Engine/Population.cpp`), never cached, so the answer is the same in the UI and in logic and on every machine. Nothing needs saving.
- `Player::canBuildMoreOfType` is asked by `BuildAssistant::canMakeUnit` (buttons, `ProductionUpdate::queueCreateUnit`), `Player::canBuild` and `BuildAssistant::isPossibleToMakeUnit` (the AI's `findFactory`), so one veto covers the player and the AI.
- A refused unit shows the engine's "maximum number" text (`CANMAKE_MAXED_OUT_FOR_PLAYER`); the tooltip line added by this slice says it is the population cap.
- Population needs sectors: with no sectors (the campaign) nothing is counted, refused or taken.
