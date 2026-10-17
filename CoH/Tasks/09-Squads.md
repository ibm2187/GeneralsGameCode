# 09 Squads

**Branch** `coh/squads` · **Needs** 00 · **Slice** `CoH/Features/Squads/` · **Flag** `Squads`

## Goal

Infantry fights as squads: one selection, one order, members spread out in formation.

## Behavior

- Each infantry type is built as a squad of N, set in INI per type.
- Clicking any member selects the squad. Orders go to the whole squad.
- Members take spread-out positions and use cover when Cover is on.
- The squad counts as one unit for transports, garrisons and population.
- Its health bar is the squad total, with a member count.
- Hero units (commandos) stay single, or are a squad of 1 with an escort, as set in INI.
- The AI builds and orders squads as units.

## Done

Domain tests cover the formation slots and member bookkeeping. Squads move, fight, garrison and ride transports in game.
