# Pacing

**Flag** `Pacing` · **Task** [03b](../../Docs/Tasks/03b-Pacing.md) · **Pairs with** [Sectors](../Sectors/README.md), [Upkeep](../Upkeep/README.md) · **Public header** [Pacing.h](Pacing.h)

Fewer, more valuable units, CoH style. Units cost more and take longer to build, and the skirmish AI's handicap follows its difficulty, so armies stay small and every loss is felt. It works without Sectors too, on the retail economy.

## How it plays

- **Prices and build times** are a share of retail, by class: infantry, vehicles and aircraft ×1.5 cost and ×1.5 build time; structures unchanged. A Ranger costs $340 and takes 7.5 s (retail $225, 5 s), a Ranger squad $1,215, a Crusader $1,350 and 15 s, an Overlord $3,000 and 30 s, a Comanche $2,250 and 30 s.
- A helicopter counts as an aircraft. Dozers, workers, harvesters and drones (`ExemptKinds`) keep retail prices, so nobody is slowed rebuilding a base.
- An `Object` line sets one unit's own cost and build time. A general's version of a unit (`Tank_ChinaTankOverlord`) uses the plain unit's line (`ChinaTankOverlord`) unless it has its own.
- Prices are rounded to `CostStep` ($5). Every price the game shows or charges changes at once: buttons, tooltips, the queue, refunds, selling and the AI (the hub's `buildCost` and `buildTime`). With [Squads](../Squads/README.md), the squad's whole price is scaled.
- [Upkeep](../Upkeep/README.md)'s population still counts by retail price: a unit costs more without taking more room.

## Skirmish AI

- **Its handicap by difficulty** (`AIEasy`, `AIMedium`, `AIHard`): the share of its income it keeps and its build times. Easy keeps 70% and builds in 130% of the time, Medium 85% and 115%, Hard plays at 100%. It applies to every income the AI gets, after Upkeep took its share.
- **Its scripts' money** (`AIScriptMoney`, 25%): the retail skirmish scripts give the AI $2,200 at the start on every difficulty and, on Hard, $10,000, $10,000, $15,000 and $20,000 as its `_ESCALATION` counter climbs (one a 10 s, +30 once its enemy has a tech building, +50 once it has a super weapon): about $57,000 by minute 13. With sectors that was more than half of a Hard AI's money (see *Measurements*). The slice scales every `PLAYER_GIVE_MONEY` a script gives to `<This Player>` at game start; the gifts still come at the same moments.
- Otherwise the AI plays as before: it reads prices through the same hub, so it sees what it can afford.

## Settings (`CoHPacing` in [Data/INI/10_Pacing.ini](Data/INI/10_Pacing.ini))

| Setting | Default | Pack | Meaning |
|---|---|---|---|
| `InfantryCost`, `VehicleCost`, `AircraftCost`, `StructureCost` | 100% | 150%, 150%, 150%, 100% | price by class, as a share of retail |
| `InfantryBuildTime`, `VehicleBuildTime`, `AircraftBuildTime`, `StructureBuildTime` | 100% | 150%, 150%, 150%, 100% | build time by class |
| `CostStep` | 5 | 5 | prices are rounded to this many dollars |
| `ExemptKinds` | DOZER HARVESTER DRONE | same | things of these kinds keep retail prices, unless they have an `Object` line |
| `Object = <object> <cost> <time>` | none | none | one object's own cost and build time, also for the generals' versions of it |
| `AIEasy`, `AIMedium`, `AIHard = <income> <time>` | 100% 100% | 70% 130%, 85% 115%, 100% 100% | the skirmish AI's handicap by difficulty |
| `AIScriptMoney` | 100% | 25% | the share of the AI scripts' gifts of money it keeps |

### Why these numbers

- **×1.5 cost and build time** for units: with Sectors a side earns about $2,000 a minute, so a Crusader is 40 s of income instead of 27, and a lost one hurts. Build times grow by the same factor, so a factory's output falls with the price: a player who lost an army has time to react before it is back, and the AI can't refill its cap in a minute.
- **Structures unchanged:** the base is what Sectors' build rule and Upkeep's cap grow from, and making it slower only slows the opening.
- **`AIScriptMoney` 25%:** the gifts average about $4,400 a minute over the first 13 minutes. In retail that was a third to a half of a Hard AI's supply income; with sectors it was twice its sector income. A quarter, about $1,100 a minute, brings it back to about half, as in retail.
- **`AIHard` 100%:** Hard still plays at full strength, with a quarter of its old gifts. Medium and Easy never got gifts; their handicap makes them gentler by step.

## Measurements

AI against AI, Hard, America against China, `-cohCash 10000`, observed headless, on Tournament Desert and Twilight Flame (`-cohStats 60`). Income is per minute over the last three minutes, after upkeep, script gifts included; units built per minute over the same three minutes (squad members count one each); the army is the side's living infantry, vehicles and aircraft and their retail cost; population and cap are Upkeep's.

Averages over both AIs of both maps (the flags-on Tournament Desert game before Pacing was over at 7:40, so its minute 10 and 15 are Twilight Flame alone; the Twilight Flame game after was over at 10:17):

| Minute | | Retail (every flag off) | Flags on, before | Flags on + Pacing, after | After / before |
|---|---|---|---|---|---|
| 5 | income a minute | 8,924 | 6,132 | 2,589 | 42% |
| 5 | units built a minute | 11.4 | 6.3 | 1.3 | 21% |
| 5 | army: units, worth | 20.8, $15,488 | 24.2, $9,050 | 6.2, $2,494 | 26%, 28% |
| 5 | population | | 50.5 | 11.2 | 22% |
| **10** | income a minute | 9,927 | 3,889 | 2,385 | 61% |
| **10** | units built a minute | 9.1 | 4.0 | 1.5 | 38% |
| **10** | **army: units, worth** | 29.5, $21,750 | 24.5, $9,350 | 9.5, $4,038 | **39%, 43%** |
| **10** | population | | 67.0 | 17.8 | 27% |
| 15 | income a minute | 17,474 | 11,342 | 4,194 | 37% |
| 15 | units built a minute | 10.7 | 2.8 | 1.3 | 46% |
| 15 | army: units, worth | 44.5, $33,700 | 23.0, $10,775 | 10.5, $5,125 | 46%, 48% |
| 15 | population | | 67.0 | 28.5 | 43% |

- **Before:** the AIs filled their cap (about 50) by minute 3 and held 50 to 80 population from minute 5 on, refilling losses at 4 to 9 units a minute. Their income was below retail; the spikes ($17,000 to $21,000 in a minute) were the scripts' gifts, more than half of a Hard AI's money.
- **After:** an army grows over the game instead of all at once: about 10 units at minute 10, at 1 to 1.5 units a minute, near the smaller cap (about 30) by minute 15. Income is $1,600 to $4,600 a minute, about what a human holding the same ground earns, plus a quarter of the gifts.
- Population before counts dozers and after doesn't (see below), so compare armies by units and worth.
- A first pass with dozers still taking population (a $1,000 dozer took 4) left the AIs 6 units at minute 10, a quarter of before: half their cap was builders. Upkeep's pack now leaves dozers out.

Each AI, before (flags on and retail):

| run | side | min | income/min (3 min avg) | built/min (3 min avg) | army units | army worth | population (cap, upkeep) |
|---|---|---|---|---|---|---|---|
| off_TournamentDesert | America | 5 | 8066 | 8.7 | 11 | 12000 | - |
| off_TournamentDesert | China | 5 | 9000 | 13.7 | 27 | 17900 | - |
| off_TournamentDesert | America | 10 | 9926 | 8.7 | 20 | 13025 | - |
| off_TournamentDesert | China | 10 | 8107 | 9.0 | 43 | 32350 | - |
| off_TournamentDesert | America | 15 | 19161 | 12.3 | 46 | 30925 | - |
| off_TournamentDesert | China | 15 | 15216 | 11.0 | 58 | 48100 | - |
| off_TwilightFlame | America | 5 | 7466 | 9.3 | 13 | 14150 | - |
| off_TwilightFlame | China | 5 | 11166 | 14.0 | 32 | 17900 | - |
| off_TwilightFlame | America | 10 | 8006 | 7.3 | 12 | 8525 | - |
| off_TwilightFlame | China | 10 | 13668 | 11.3 | 43 | 33100 | - |
| off_TwilightFlame | America | 15 | 13406 | 9.3 | 14 | 8675 | - |
| off_TwilightFlame | China | 15 | 22114 | 10.3 | 60 | 47100 | - |
| on_TournamentDesert | America | 5 | 5991 | 5.7 | 28 | 10625 | 54 (54, 10%) |
| on_TournamentDesert | China | 5 | 6805 | 9.0 | 24 | 8700 | 50 (52, 10%) |
| on_TwilightFlame | America | 5 | 5565 | 4.3 | 18 | 7275 | 50 (52, 10%) |
| on_TwilightFlame | China | 5 | 6168 | 6.3 | 27 | 9600 | 48 (49, 10%) |
| on_TwilightFlame | America | 10 | 4002 | 3.3 | 25 | 11300 | 80 (49, 25%) |
| on_TwilightFlame | China | 10 | 3776 | 4.7 | 24 | 7400 | 54 (61, 10%) |
| on_TwilightFlame | America | 15 | 9998 | 1.3 | 22 | 9075 | 73 (58, 25%) |
| on_TwilightFlame | China | 15 | 12687 | 4.3 | 24 | 12475 | 61 (64, 25%) |

After (flags on + Pacing):

| run | side | min | income/min (3 min avg) | built/min (3 min avg) | army units | army worth | population (cap, upkeep) |
|---|---|---|---|---|---|---|---|
| on_TournamentDesert | America | 5 | 2733 | 1.0 | 0 | 0 | 0 (34, 0%) |
| on_TournamentDesert | China | 5 | 2606 | 2.0 | 9 | 3200 | 9 (27, 0%) |
| on_TournamentDesert | America | 10 | 1621 | 1.0 | 8 | 2250 | 9 (30, 0%) |
| on_TournamentDesert | China | 10 | 3107 | 3.0 | 19 | 6525 | 27 (38, 10%) |
| on_TournamentDesert | America | 15 | 3750 | 1.3 | 10 | 4325 | 27 (34, 10%) |
| on_TournamentDesert | China | 15 | 4638 | 1.3 | 11 | 5925 | 30 (32, 10%) |
| on_TwilightFlame | America | 5 | 2365 | 1.3 | 13 | 5375 | 27 (31, 10%) |
| on_TwilightFlame | China | 5 | 2653 | 1.0 | 3 | 1400 | 9 (29, 0%) |
| on_TwilightFlame | America | 10 | 3133 | 0.7 | 11 | 7375 | 35 (33, 25%) |
| on_TwilightFlame | China | 10 | 1679 | 1.3 | 0 | 0 | 0 (23, 0%) |

## Hooks

| Hub point | Used for |
|---|---|
| `buildCost` (`ThingTemplate::calcCostToBuild`) | the price by class or `Object` line |
| `buildTime` (`ThingTemplate::calcTimeToBuild`) | the build time, times the AI's handicap |
| `income` (`Money::deposit`) | the AI's share of its income |
| `gameStartAfterMap` | scales the AI scripts' gifts of money |

No new hook points in original code.

## Files

| File | What |
|---|---|
| `Pacing.h` | public: `isActive`, `costMultiplier`, `buildTimeMultiplier`, `incomeMultiplier` |
| `Domain/Pacing` | the class of a thing, the plain unit of a general's version, the multiplier that applies, rounding, income scaling |
| `Engine/Pacing.cpp` | the slice, the `CoHPacing` block, the hub subscribers, the public answers |
| `Engine/AIScripts.cpp` | scales the scripts' `PLAYER_GIVE_MONEY` to `<This Player>` at game start |
| `Tests/` | classes, base names, own lines, rounding, income |

## Engine notes

- `calcCostToBuild` and `calcTimeToBuild` are what the buttons, tooltips, `ProductionUpdate`'s queue and refunds, `BuildAssistant` (selling) and the AI's affordability checks all ask. Squads subscribes first (the slice list order in `CoH/CMakeLists.txt`), so Pacing scales the whole squad.
- The skirmish AI's money cheats live in `SkirmishScripts.scb` as one-shot scripts named "<Side> AI Bonus Early/Mid/Late/Really Late" (Hard only) and "<Side> Give AI money to cover bug" (all difficulties), with a `PLAYER_GIVE_MONEY` action (`ScriptActions::doGiveMoney`: a tracked `Money::deposit`, so it counts as income and pays upkeep). They fire on the `_ESCALATION` counter, which "Time Escalation - H" raises by one every 10 s; "Tech Center Escalation" and "Super Weapon Escalation" add 30 and 50 once.
- A save keeps only which scripts are active (`Script::xfer`); the scripts themselves are read again from the map and `SkirmishScripts.scb` on load, so scaling them at every game start never scales twice.
- Starting money is deposited untracked (`Player::init`), but a map's `playerStartMoney` is tracked (`Player::initFromDict`). The $10,000 "income" in a Hard AI's first minute is the "AI Bonus Early" gift.
- `-cohStats` (the Kernel's `Engine/Stats.cpp`) counts income after every feature took its share, with any flags, so it measures retail too.
