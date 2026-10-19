# 06 Suppression (postponed)

**Postponed 2026-10-19** by the user; not cancelled. Pick it up again only when the user asks.

**Branch** `coh/suppression` · **Needs** 04 · **Slice** `CoH/Features/Suppression/` · **Flag** `Suppression`

## Goal

Machine guns and explosions pin infantry, so firepower controls space instead of only killing.

## Behavior

- Weapons have a suppression value set in INI: high for machine guns, some for explosives.
- Hits and near misses fill a target's suppression meter, which drains over time.
- **Suppressed:** slower movement, worse accuracy and a slower rate of fire.
- **Pinned:** the unit can't move except to retreat, and fires rarely.
- Vehicles aren't suppressed. Squads garrisoned in a trench (14) or a building take less.
- An icon over the unit shows its state.
- The AI pulls pinned infantry back and flanks machine-gun positions.

## Done

Domain tests cover the meter, the thresholds and the decay. The states are visible in game. A machine gun stops an infantry rush at range.
