# Upkeep

**Flag** `Upkeep` · **Task** [03](../../Docs/Tasks/03-Upkeep.md) · **Needs** [Sectors](../Sectors/README.md) · **Public header** [Upkeep.h](Upkeep.h)

A big army costs money to keep. Fewer, better units pay off; spamming units doesn't.

## How it plays

- **Population.** Each unit takes population by its build cost: one per `BuildCostPerPopulation` (250), rounded, at least `MinUnitPopulation`. A Ranger takes 1, a Crusader 4, an Overlord 8. Units that cost nothing (called-in planes, spawned helpers) and drones take none. Units in production queues count too, so the queue can't overshoot the cap. With [Squads](../Squads/README.md) a squad counts once, by its whole price (a Ranger squad, 810, takes 3), through `Squads.h`.
- **The cap** starts low and rises with buildings and with sectors held: `CapBase` (10), plus each finished building of a `CapBuilding` kind (command center 10, barracks 4, war factory 6, airfield 4), plus `CapPerSector` (3) for each sector held, up to `CapMax` (150). A start with a command center and the home sector has 23.
- A unit that doesn't fit under the cap can't be made: its button is greyed and its tooltip says why. Workers and dozers (`NeverRefusedKinds`) take population but are always made, so nobody loses their builders. Units that come some other way (paradrops, ambushes) still count, even past the cap.
- **Upkeep** takes a share of every income, growing with population in steps (`UpkeepStep`): nothing below 30, 10% from 30, 25% from 60, 40% from 100. With Sectors almost all income is sector income; oil derricks, hackers and the like pay upkeep too. Starting money, selling and refunds don't.
- **On screen:** the population is next to the money ("$ 2400   Pop 34/60 (-10%)"). The money tooltip shows the income per minute as gross, upkeep and net, the population and cap, and the next step. A unit's build tooltip shows the population it takes.

## Skirmish AI

The AI plays under the same cap, and grows its army only while upkeep stays at or below `AIMaxUpkeepPercent` (25%): it stops before the 40% step, so most of its income stays free for rebuilding and its economy never stalls. A unit it can't make makes its factory search fail, so it picks other teams; teams it couldn't fill expire as they always did.

## Settings (`CoHUpkeep` in [Data/INI/10_Upkeep.ini](Data/INI/10_Upkeep.ini))

| Setting | Default | Meaning |
|---|---|---|
| `BuildCostPerPopulation` | 250 | a unit takes one population per this much of its build cost, rounded |
| `MinUnitPopulation` | 1 | a unit that costs money takes at least this |
| `Population = <object> <n>` | none | one unit's population, instead of by its cost |
| `UnitKinds` | INFANTRY VEHICLE AIRCRAFT | objects of any of these kinds take population... |
| `NonUnitKinds` | STRUCTURE DRONE | ...unless they are any of these |
| `NeverRefusedKinds` | DOZER HARVESTER | take population but are made even at the cap |
| `CapBase` | 10 | every player's cap starts here |
| `CapPerSector` | 3 | each sector held adds this |
| `CapMax` | 150 | the cap never goes higher |
| `CapBuilding = <KINDOF> <n>` | see the pack | each finished building of the kind adds n |
| `UpkeepStep = <population> <percent>` | see the pack | from that population on, that share of income is upkeep |
| `AIMaxUpkeepPercent` | 25 | the AI grows its army only while upkeep stays at or below this |

The pack's `Population`, `CapBuilding` and `UpkeepStep` lines replace the defaults rather than adding to them.

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
