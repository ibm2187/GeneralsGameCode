# 03c Resources

**Branch** `coh/resources` · **Needs** 01, 02, 03, 03b · **Slice** `CoH/Features/Resources/` · **Flag** `Resources`

## Goal

CoH's three resources instead of one pile of money. Manpower pays for everything, but tanks need fuel and upgrades need munitions, and both come only from ground you hold. A big opening is impossible: armies grow with the map you control. Command points, earned in combat, unlock tech.

Decided with the user (2026-10-17), after task 03b's play-test: a Hard GLA AI had two or three tanks, two Quads and infantry while the user had taken two points. The cause was a single resource with a large start ($10,000, five minutes of income), not the income rate.

## Behavior

- **Manpower** is the engine's money, shown as manpower. Prices, refunds, selling and the AI's affordability checks stay as they are.
  - Income is mostly a flat base rate per player (Sectors' home income), cut by Upkeep, plus a small bonus per sector held.
  - Starting resources are small, CoH style: a barracks and a squad or two, not a base and an army.
- **Fuel** comes from fuel points: sectors with a tech building (Sectors' high-value points), and others the map's layout makes fuel. Vehicles, aircraft and tech structures (war factory, airfield, the tech buildings) cost fuel as well as manpower.
- **Munitions** come from munitions points (the former supply piles). Upgrades cost munitions; unit abilities (task 13) will too.
- Each sector point is one type: fuel or munitions, set by rule, with an INI override per map. Its income table has one row per type and resource.
- **Command points** stay GeneralsArsenal's, earned in combat, and keep paying for powers. Spending them also unlocks tech: unit tiers or upgrades, CoH commander style (unlocks set in INI; Doctrines, task 16, builds its trees on this).
- **Costs** per object and upgrade in INI (`Cost = <object> <fuel> <munitions>`), with defaults by class (Pacing's classes). A build that can't be paid is greyed, with the missing resource in the tooltip.
- **On screen:** manpower, fuel and munitions next to each other where the money is, each with its income a minute; the tooltip splits income by source and upkeep.
- **The skirmish AI** pays the same costs. It weighs fuel and munitions points by what it is short of when choosing sectors to take (Sectors' AI targets), skips builds it can't pay, and spends command points on unlocks.

## Hook points

Charging and refunding the new resources where the engine takes and gives back money: `ProductionUpdate::queueCreateUnit` and its cancel, the dozer's structure placement and its cancel, upgrades queued and cancelled, selling. Look for one general hub point per engine event (for example "a build was paid / refunded"), next to the existing `production` veto (affordability), `buildCost`, `income`, `moneyDisplay`, `moneyTooltip` and `commandTooltip`. The per-player stores are saved (`registerSaveData`) and part of the game state.

## Done

- Domain tests cover the stores, costs, income by sector type and unlocks.
- Replays play clean with the flag off and on; a saved game keeps every resource.
- Measured with `-cohStats` (extended to the three resources) as in 03b: the Hard AI's first vehicle comes no earlier than minute 4 and its army at minute 10 is no bigger than in 03b.
- The user plays a windowed skirmish against a Hard AI of each faction and finds it fair.
- READMEs, the hooks table and CONTEXT.md are updated.
